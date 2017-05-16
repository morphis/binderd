/*
 * Copyright (C) 2017 Simon Fels <morphis@gravedo.de>
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

#include <binder/IPCThreadStateNew.h>

static pthread_mutex_t gTLSMutex = PTHREAD_MUTEX_INITIALIZER;
static bool gHaveTLS = false;
static pthread_key_t gTLS = 0;
static bool gShutdown = false;
static bool gDisableBackgroundScheduling = false;

namespace android {
IPCThreadStateNew* IPCThreadStateNew::self() {
  if (gHaveTLS) {
restart:
    const pthread_key_t k = gTLS;
    IPCThreadStateNew* st = (IPCThreadStateNew*)pthread_getspecific(k);
    if (st) return st;
    return new IPCThreadStateNew;
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

IPCThreadStateNew* IPCThreadStateNew::selfOrNull() {
  if (gHaveTLS) {
    const pthread_key_t k = gTLS;
    IPCThreadStateNew* st = (IPCThreadStateNew*)pthread_getspecific(k);
    return st;
  }
  return NULL;
}

void IPCThreadStateNew::threadDestructor(void *st) {
  IPCThreadStateNew* const self = static_cast<IPCThreadStateNew*>(st);
  if (self) {
    self->flushCommands();
    delete self;
  }
}

sp<ProcessState> IPCThreadStateNew::process() {
  return mProcess;
}

status_t IPCThreadStateNew::clearLastError() {
  return OK;
}

pid_t IPCThreadStateNew::getCallingPid() const {
  return mCallingPid;
}

uid_t IPCThreadStateNew::getCallingUid() const {
  return mCallingUid;
}

void IPCThreadStateNew::setStrictModePolicy(int32_t policy) {
  mStrictModePolicy = policy;
}

int32_t IPCThreadStateNew::getStrictModePolicy() const {
  return mStrictModePolicy;
}

void IPCThreadStateNew::setLastTransactionBinderFlags(int32_t flags) {
  mLastTransactionBinderFlags = flags;
}

int32_t IPCThreadStateNew::getLastTransactionBinderFlags() const {
  return mLastTransactionBinderFlags;
}

int64_t IPCThreadStateNew::clearCallingIdentity() {
  int64_t token = (static_cast<int64_t>(mCallingUid) << 32) | mCallingPid;
  clearCaller();
  return token;
}

void IPCThreadStateNew::clearCaller() {
  mCallingPid = getpid();
  mCallingUid = getuid();
}

void IPCThreadStateNew::restoreCallingIdentity(int64_t token) {
  mCallingUid = static_cast<int>(token >> 32);
  mCallingPid = static_cast<int>(token);
}

int IPCThreadStateNew::setupPolling(int* fd) {
  return 0;
}

status_t IPCThreadStateNew::handlePolledCommands() {
  return OK;
}

void IPCThreadStateNew::flushCommands() {
}

void IPCThreadStateNew::joinThreadPool(bool isMain) {
}

void IPCThreadStateNew::stopProcess(bool immediate) {
}

status_t IPCThreadStateNew::transact(int32_t handle, uint32_t code,
                  const Parcel& data,
                  Parcel* reply, uint32_t flags) {
  return OK;
}

void IPCThreadStateNew::incStrongHandle(int32_t handle) {

}

void IPCThreadStateNew::decStrongHandle(int32_t handle) {

}

void IPCThreadStateNew::incWeakHandle(int32_t handle) {

}

void IPCThreadStateNew::decWeakHandle(int32_t handle) {

}

status_t IPCThreadStateNew::attemptIncStrongHandle(int32_t handle) {
  return OK;
}

void IPCThreadStateNew::expungeHandle(int32_t handle, IBinder* binder) {

}

status_t IPCThreadStateNew::requestDeathNotification(int32_t handle, BpBinder* proxy) {
  return OK;
}

status_t IPCThreadStateNew::clearDeathNotification(int32_t handle, BpBinder* proxy) {
  return OK;
}

void IPCThreadStateNew::shutdown() {
  gShutdown = true;

  if (gHaveTLS) {
    // XXX Need to wait for all thread pool threads to exit!
    IPCThreadStateNew* st = (IPCThreadStateNew*)pthread_getspecific(gTLS);
    if (st) {
      delete st;
      pthread_setspecific(gTLS, NULL);
    }
    gHaveTLS = false;
  }
}

// Call this to disable switching threads to background scheduling when
// receiving incoming IPC calls.  This is specifically here for the
// Android system process, since it expects to have background apps calling
// in to it but doesn't want to acquire locks in its services while in
// the background.
void IPCThreadStateNew::disableBackgroundScheduling(bool disable) {
  gDisableBackgroundScheduling = disable;
}

// Call blocks until the number of executing binder threads is less than
// the maximum number of binder threads threads allowed for this process.
void IPCThreadStateNew::blockUntilThreadAvailable() {}

IPCThreadStateNew::IPCThreadStateNew() :
  mProcess(ProcessState::self()),
  mStrictModePolicy(0),
  mLastTransactionBinderFlags(0) {
  pthread_setspecific(gTLS, this);
  clearCaller();
}

IPCThreadStateNew::~IPCThreadStateNew() {}
}
