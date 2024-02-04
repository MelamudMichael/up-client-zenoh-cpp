// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: uuid.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_uuid_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_uuid_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3021000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3021012 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_uuid_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_uuid_2eproto {
  static const uint32_t offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_uuid_2eproto;
namespace uprotocol {
namespace v1 {
class UUID;
struct UUIDDefaultTypeInternal;
extern UUIDDefaultTypeInternal _UUID_default_instance_;
}  // namespace v1
}  // namespace uprotocol
PROTOBUF_NAMESPACE_OPEN
template<> ::uprotocol::v1::UUID* Arena::CreateMaybeMessage<::uprotocol::v1::UUID>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace uprotocol {
namespace v1 {

// ===================================================================

class UUID final :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:uprotocol.v1.UUID) */ {
 public:
  inline UUID() : UUID(nullptr) {}
  ~UUID() override;
  explicit PROTOBUF_CONSTEXPR UUID(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  UUID(const UUID& from);
  UUID(UUID&& from) noexcept
    : UUID() {
    *this = ::std::move(from);
  }

  inline UUID& operator=(const UUID& from) {
    CopyFrom(from);
    return *this;
  }
  inline UUID& operator=(UUID&& from) noexcept {
    if (this == &from) return *this;
    if (GetOwningArena() == from.GetOwningArena()
  #ifdef PROTOBUF_FORCE_COPY_IN_MOVE
        && GetOwningArena() != nullptr
  #endif  // !PROTOBUF_FORCE_COPY_IN_MOVE
    ) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return default_instance().GetMetadata().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return default_instance().GetMetadata().reflection;
  }
  static const UUID& default_instance() {
    return *internal_default_instance();
  }
  static inline const UUID* internal_default_instance() {
    return reinterpret_cast<const UUID*>(
               &_UUID_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(UUID& a, UUID& b) {
    a.Swap(&b);
  }
  inline void Swap(UUID* other) {
    if (other == this) return;
  #ifdef PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() != nullptr &&
        GetOwningArena() == other->GetOwningArena()) {
   #else  // PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() == other->GetOwningArena()) {
  #endif  // !PROTOBUF_FORCE_COPY_IN_SWAP
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(UUID* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  UUID* New(::PROTOBUF_NAMESPACE_ID::Arena* arena = nullptr) const final {
    return CreateMaybeMessage<UUID>(arena);
  }
  using ::PROTOBUF_NAMESPACE_ID::Message::CopyFrom;
  void CopyFrom(const UUID& from);
  using ::PROTOBUF_NAMESPACE_ID::Message::MergeFrom;
  void MergeFrom( const UUID& from) {
    UUID::MergeImpl(*this, from);
  }
  private:
  static void MergeImpl(::PROTOBUF_NAMESPACE_ID::Message& to_msg, const ::PROTOBUF_NAMESPACE_ID::Message& from_msg);
  public:
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  uint8_t* _InternalSerialize(
      uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _impl_._cached_size_.Get(); }

  private:
  void SharedCtor(::PROTOBUF_NAMESPACE_ID::Arena* arena, bool is_message_owned);
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(UUID* other);

  private:
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "uprotocol.v1.UUID";
  }
  protected:
  explicit UUID(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  public:

  static const ClassData _class_data_;
  const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*GetClassData() const final;

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kMsbFieldNumber = 1,
    kLsbFieldNumber = 2,
  };
  // fixed64 msb = 1;
  void clear_msb();
  uint64_t msb() const;
  void set_msb(uint64_t value);
  private:
  uint64_t _internal_msb() const;
  void _internal_set_msb(uint64_t value);
  public:

  // fixed64 lsb = 2;
  void clear_lsb();
  uint64_t lsb() const;
  void set_lsb(uint64_t value);
  private:
  uint64_t _internal_lsb() const;
  void _internal_set_lsb(uint64_t value);
  public:

  // @@protoc_insertion_point(class_scope:uprotocol.v1.UUID)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  struct Impl_ {
    uint64_t msb_;
    uint64_t lsb_;
    mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  };
  union { Impl_ _impl_; };
  friend struct ::TableStruct_uuid_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// UUID

// fixed64 msb = 1;
inline void UUID::clear_msb() {
  _impl_.msb_ = uint64_t{0u};
}
inline uint64_t UUID::_internal_msb() const {
  return _impl_.msb_;
}
inline uint64_t UUID::msb() const {
  // @@protoc_insertion_point(field_get:uprotocol.v1.UUID.msb)
  return _internal_msb();
}
inline void UUID::_internal_set_msb(uint64_t value) {
  
  _impl_.msb_ = value;
}
inline void UUID::set_msb(uint64_t value) {
  _internal_set_msb(value);
  // @@protoc_insertion_point(field_set:uprotocol.v1.UUID.msb)
}

// fixed64 lsb = 2;
inline void UUID::clear_lsb() {
  _impl_.lsb_ = uint64_t{0u};
}
inline uint64_t UUID::_internal_lsb() const {
  return _impl_.lsb_;
}
inline uint64_t UUID::lsb() const {
  // @@protoc_insertion_point(field_get:uprotocol.v1.UUID.lsb)
  return _internal_lsb();
}
inline void UUID::_internal_set_lsb(uint64_t value) {
  
  _impl_.lsb_ = value;
}
inline void UUID::set_lsb(uint64_t value) {
  _internal_set_lsb(value);
  // @@protoc_insertion_point(field_set:uprotocol.v1.UUID.lsb)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace v1
}  // namespace uprotocol

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_uuid_2eproto