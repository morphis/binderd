/*
 * Copyright (C) 2005 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "IPCThreadState"

#include <binder/IPCThreadState.h>

#include <binder/Binder.h>
#include <binder/BpBinder.h>
#include <binder/TextOutput.h>

#include <cutils/sched_policy.h>
#include <utils/Log.h>
#include <utils/SystemClock.h>
#include <utils/threads.h>

#include <private/binder/binder_module.h>
#include <private/binder/Static.h>

#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/types.h>
#include <asm-generic/unistd.h>

#include "binderd/logger.h"
#include "binderd/client.h"

#include "TransactionDataFromParcel.h"

// ---------------------------------------------------------------------------

namespace android {
static pid_t gettid() {
    pid_t id = syscall(__NR_gettid);
    return id;
}

static pthread_mutex_t gTLSMutex = PTHREAD_MUTEX_INITIALIZER;
static bool gHaveTLS = false;
static pthread_key_t gTLS = 0;
static bool gShutdown = false;
static bool gDisableBackgroundScheduling = false;

IPCThreadState* IPCThreadState::self()
{
    if (gHaveTLS) {
restart:
        const pthread_key_t k = gTLS;
        IPCThreadState* st = (IPCThreadState*)pthread_getspecific(k);
        if (st) return st;
        return new IPCThreadState;
    }
    
    if (gShutdown) {
        ALOGW("Calling IPCThreadState::self() during shutdown is dangerous, expect a crash.\n");
        return NULL;
    }
    
    pthread_mutex_lock(&gTLSMutex);
    if (!gHaveTLS) {
        int key_create_value = pthread_key_create(&gTLS, threadDestructor);
        if (key_create_value != 0) {
            pthread_mutex_unlock(&gTLSMutex);
            ALOGW("IPCThreadState::self() unable to create TLS key, expect a crash: %s\n",
                    strerror(key_create_value));
            return NULL;
        }
        gHaveTLS = true;
    }
    pthread_mutex_unlock(&gTLSMutex);
    goto restart;
}

IPCThreadState* IPCThreadState::selfOrNull()
{
    if (gHaveTLS) {
        const pthread_key_t k = gTLS;
        IPCThreadState* st = (IPCThreadState*)pthread_getspecific(k);
        return st;
    }
    return NULL;
}

void IPCThreadState::shutdown()
{
    gShutdown = true;
    
    if (gHaveTLS) {
        // XXX Need to wait for all thread pool threads to exit!
        IPCThreadState* st = (IPCThreadState*)pthread_getspecific(gTLS);
        if (st) {
            delete st;
            pthread_setspecific(gTLS, NULL);
        }
        gHaveTLS = false;
    }
}

void IPCThreadState::disableBackgroundScheduling(bool disable)
{
    gDisableBackgroundScheduling = disable;
}

sp<ProcessState> IPCThreadState::process()
{
    return mProcess;
}

status_t IPCThreadState::clearLastError()
{
    const status_t err = mLastError;
    mLastError = NO_ERROR;
    return err;
}

pid_t IPCThreadState::getCallingPid() const
{
    return mCallingPid;
}

uid_t IPCThreadState::getCallingUid() const
{
    return mCallingUid;
}

int64_t IPCThreadState::clearCallingIdentity()
{
    int64_t token = ((int64_t)mCallingUid<<32) | mCallingPid;
    clearCaller();
    return token;
}

void IPCThreadState::setStrictModePolicy(int32_t policy)
{
    mStrictModePolicy = policy;
}

int32_t IPCThreadState::getStrictModePolicy() const
{
    return mStrictModePolicy;
}

void IPCThreadState::setLastTransactionBinderFlags(int32_t flags)
{
    mLastTransactionBinderFlags = flags;
}

int32_t IPCThreadState::getLastTransactionBinderFlags() const
{
    return mLastTransactionBinderFlags;
}

void IPCThreadState::restoreCallingIdentity(int64_t token)
{
    mCallingUid = (int)(token>>32);
    mCallingPid = (int)token;
}

void IPCThreadState::clearCaller()
{
    mCallingPid = getpid();
    mCallingUid = getuid();
}

void IPCThreadState::flushCommands()
{
}

void IPCThreadState::blockUntilThreadAvailable()
{
#if 0
    pthread_mutex_lock(&mProcess->mThreadCountLock);
    while (mProcess->mExecutingThreadsCount >= mProcess->mMaxThreads) {
        ALOGW("Waiting for thread to be free. mExecutingThreadsCount=%lu mMaxThreads=%lu\n",
                static_cast<unsigned long>(mProcess->mExecutingThreadsCount),
                static_cast<unsigned long>(mProcess->mMaxThreads));
        pthread_cond_wait(&mProcess->mThreadCountDecrement, &mProcess->mThreadCountLock);
    }
    pthread_mutex_unlock(&mProcess->mThreadCountLock);
#endif
}

void IPCThreadState::joinThreadPool(bool isMain)
{
  (void) isMain;
  do {
    if (!mProcess->mClient->ProcessAndExecuteCommand())
      break;
  } while (true);
}

int IPCThreadState::setupPolling(int* fd)
{
    return 0;
}

status_t IPCThreadState::handlePolledCommands()
{
  return OK;
}

void IPCThreadState::stopProcess(bool /*immediate*/)
{
}

status_t IPCThreadState::transact(int32_t handle,
                                  uint32_t code, const Parcel& data,
                                  Parcel* reply, uint32_t flags)
{
  auto err = data.errorCheck();
  if (err != NO_ERROR) {
    reply->setError(err);
    return err;
  }

  auto request = std::make_unique<TransactionDataFromParcel>(mProcess->mClient, code, &data, flags & TF_ONE_WAY);
  std::unique_ptr<binderd::TransactionData> response;
  auto status = static_cast<status_t>(mProcess->mClient->Transact(handle, std::move(request), &response));
  if (status != OK) {
    if (reply)
      reply->setError(status);
    return status;
  }

  if (response)
    android::InitParcelFromTransactionData(*reply, response);

  return OK;
}

void IPCThreadState::incStrongHandle(int32_t handle)
{
  mProcess->mClient->AddReference(handle);
}

void IPCThreadState::decStrongHandle(int32_t handle)
{
  mProcess->mClient->ReleaseReference(handle);
}

void IPCThreadState::incWeakHandle(int32_t handle)
{
}

void IPCThreadState::decWeakHandle(int32_t handle)
{
}

status_t IPCThreadState::attemptIncStrongHandle(int32_t handle)
{
    (void)handle;
    ALOGE("%s(%d): Not supported\n", __func__, handle);
    return INVALID_OPERATION;
}

void IPCThreadState::expungeHandle(int32_t handle, IBinder* binder)
{
    self()->mProcess->expungeHandle(handle, binder);
}

status_t IPCThreadState::requestDeathNotification(int32_t handle, BpBinder* proxy)
{
    return NO_ERROR;
}

status_t IPCThreadState::clearDeathNotification(int32_t handle, BpBinder* proxy)
{
    return NO_ERROR;
}

IPCThreadState::IPCThreadState()
    : mProcess(ProcessState::self()),
      mMyThreadId(gettid()),
      mStrictModePolicy(0),
      mLastTransactionBinderFlags(0)
{
    pthread_setspecific(gTLS, this);
    clearCaller();
}

IPCThreadState::~IPCThreadState()
{
}

void IPCThreadState::threadDestructor(void *st)
{
    IPCThreadState* const self = static_cast<IPCThreadState*>(st);
    if (self) {
        self->flushCommands();
        delete self;
    }
}
}; // namespace android
