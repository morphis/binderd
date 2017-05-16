/*
 * Copyright (C) 2016 Simon Fels <morphis@gravedo.de>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "TransactionDataFromParcel.h"

#include "binder/BpBinder.h"

#include "binderd/logger.h"
#include "binderd/common/utils.h"
#include "binderd/common/binary_writer.h"
#include "binderd/object.h"
#include "binderd/client.h"

#include <memory.h>

namespace {
// WrappedObject is a tiny wrapper around an IBinder based object
// implementation which maps it's semantics into a binderd::Object
// implementation which is required by libbinderd.
class WrappedObject : public binderd::Object {
 public:
  WrappedObject(const uintptr_t &cookie) : cookie_{cookie} {}
  ~WrappedObject() override {}

  Type GetType() const override { return Type::Local; }

  binderd::Status Transact(const binderd::ClientPtr &client,
                           std::unique_ptr<binderd::TransactionData> request,
                           std::unique_ptr<binderd::TransactionData> *reply) override {

    android::Parcel data, response;

    android::InitParcelFromTransactionData(data, request);

    auto binder = reinterpret_cast<android::IBinder*>(cookie_);
    auto status = static_cast<binderd::Status>(binder->transact(request->GetCode(), data, &response));
    if (!reply)
      return status;

    *reply = std::make_unique<android::TransactionDataFromParcel>(client.get(), request->GetCode(), &response);

    const auto err = response.errorCheck();
    if (err != android::NO_ERROR)
      return static_cast<binderd::Status>(err);

    return status;
  }

  binderd::Status LinkToDeath(const binderd::ClientPtr &client, const std::shared_ptr<DeathRecipient> &recipient) override {
    (void) client;
    (void) recipient;
    return binderd::Status::InvalidOperation;
  }

  binderd::Status UnlinkToDeath(const binderd::ClientPtr &client, const std::shared_ptr<DeathRecipient> &recipient) override {
    (void) client;
    (void) recipient;
    return binderd::Status::InvalidOperation;
  }

  void OnReferenceAcquired() {
    auto binder = reinterpret_cast<android::BpBinder*>(cookie_);
    binder->incStrong(this);
  }

  void OnReferenceRelased() {
    auto binder = reinterpret_cast<android::BpBinder*>(cookie_);
    binder->decStrong(this);
  }

 private:
  uintptr_t cookie_;
};
}

namespace android {
void InitParcelFromTransactionData(android::Parcel &p, const std::unique_ptr<binderd::TransactionData> &data) {
  p.ipcSetDataReference(reinterpret_cast<const uint8_t*>(data->GetData()), data->GetDataSize(),
                        reinterpret_cast<const binder_size_t*>(data->GetObjectOffsets()), data->GetNumObjectOffsets(),
                        // We don't give a function to free up space here as it will be still
                        // owned by the transaction
                        [](android::Parcel* parcel, const uint8_t* data, size_t data_size,
                           const binder_size_t* objects, size_t objects_size, void* cookie) {
    (void) parcel;
    (void) data;
    (void) data_size;
    (void) objects;
    (void) objects_size;
    (void) cookie;
  }, nullptr);
}

TransactionDataFromParcel::TransactionDataFromParcel(
    binderd::Client *client, const uint32_t &code, const Parcel *parcel, bool one_way) : code_{code}, one_way_{one_way} {

  data_ = binderd::CreateBufferWithSize(parcel->ipcDataSize());
  ::memcpy(data_->GetData(),
           reinterpret_cast<uint8_t*>(parcel->ipcData()),
           data_->GetSize());

  objects_ = binderd::CreateBufferWithSize(parcel->ipcObjectsCount() * sizeof(binder_size_t));
  ::memcpy(objects_->GetData(),
           reinterpret_cast<uint8_t*>(parcel->ipcObjects()),
           objects_->GetSize());

  if (!client)
    return;

  auto object_pool = client->GetObjectPool();

  auto current = objects_->GetData();
  while (current != objects_->GetData() + objects_->GetSize()) {
    auto offset = static_cast<binder_size_t>(*current);
    auto obj = reinterpret_cast<flat_binder_object*>(data_->GetData() + offset);

    switch (obj->type) {
    case BINDER_TYPE_BINDER: {
      auto wrapped_obj = std::shared_ptr<binderd::Object>(new WrappedObject(obj->cookie));
      const auto cookie = reinterpret_cast<uintptr_t>(wrapped_obj.get());
      obj->cookie = cookie;
      if (!object_pool->Contains(cookie))
        object_pool->AdoptObject(wrapped_obj);
      break;
    }
    default:
      break;
    }

    current += sizeof(binder_size_t);
  }
}

TransactionDataFromParcel::~TransactionDataFromParcel() {}

uint32_t TransactionDataFromParcel::GetCode() const {
  return code_;
}

uintptr_t TransactionDataFromParcel::GetBinder() const {
  return 0;
}

uintptr_t TransactionDataFromParcel::GetCookie() const {
  return 0;
}

bool TransactionDataFromParcel::IsOneWay() const {
  return one_way_;
}

bool TransactionDataFromParcel::HasStatus() const {
  return false;
}

binderd::Status TransactionDataFromParcel::GetStatus() const {
  return binderd::Status::OK;
}

const uint8_t* TransactionDataFromParcel::GetData() const {
  return GetMutableData();
}

uint8_t* TransactionDataFromParcel::GetMutableData() const {
  if (!data_)
    return nullptr;
  return data_->GetData();
}

size_t TransactionDataFromParcel::GetDataSize() const {
  if (!data_)
    return 0;
  return data_->GetSize();

}

const uint8_t* TransactionDataFromParcel::GetObjectOffsets() const {
  if (!objects_)
    return nullptr;
  return objects_->GetData();
}

size_t TransactionDataFromParcel::GetNumObjectOffsets() const {
  if (!objects_)
    return 0;
  return objects_->GetSize() / sizeof(binder_size_t);
}
} // namespace android
