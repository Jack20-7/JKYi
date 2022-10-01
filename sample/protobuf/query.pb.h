// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: query.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_query_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_query_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3021000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3021006 < PROTOBUF_MIN_PROTOC_VERSION
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
#define PROTOBUF_INTERNAL_EXPORT_query_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_query_2eproto {
  static const uint32_t offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_query_2eproto;
namespace test {
class Answer;
struct AnswerDefaultTypeInternal;
extern AnswerDefaultTypeInternal _Answer_default_instance_;
class Empty;
struct EmptyDefaultTypeInternal;
extern EmptyDefaultTypeInternal _Empty_default_instance_;
class Query;
struct QueryDefaultTypeInternal;
extern QueryDefaultTypeInternal _Query_default_instance_;
}  // namespace test
PROTOBUF_NAMESPACE_OPEN
template<> ::test::Answer* Arena::CreateMaybeMessage<::test::Answer>(Arena*);
template<> ::test::Empty* Arena::CreateMaybeMessage<::test::Empty>(Arena*);
template<> ::test::Query* Arena::CreateMaybeMessage<::test::Query>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace test {

// ===================================================================

class Query final :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:test.Query) */ {
 public:
  inline Query() : Query(nullptr) {}
  ~Query() override;
  explicit PROTOBUF_CONSTEXPR Query(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  Query(const Query& from);
  Query(Query&& from) noexcept
    : Query() {
    *this = ::std::move(from);
  }

  inline Query& operator=(const Query& from) {
    CopyFrom(from);
    return *this;
  }
  inline Query& operator=(Query&& from) noexcept {
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

  inline const ::PROTOBUF_NAMESPACE_ID::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance);
  }
  inline ::PROTOBUF_NAMESPACE_ID::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
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
  static const Query& default_instance() {
    return *internal_default_instance();
  }
  static inline const Query* internal_default_instance() {
    return reinterpret_cast<const Query*>(
               &_Query_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(Query& a, Query& b) {
    a.Swap(&b);
  }
  inline void Swap(Query* other) {
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
  void UnsafeArenaSwap(Query* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  Query* New(::PROTOBUF_NAMESPACE_ID::Arena* arena = nullptr) const final {
    return CreateMaybeMessage<Query>(arena);
  }
  using ::PROTOBUF_NAMESPACE_ID::Message::CopyFrom;
  void CopyFrom(const Query& from);
  using ::PROTOBUF_NAMESPACE_ID::Message::MergeFrom;
  void MergeFrom( const Query& from) {
    Query::MergeImpl(*this, from);
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
  void InternalSwap(Query* other);

  private:
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "test.Query";
  }
  protected:
  explicit Query(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  public:

  static const ClassData _class_data_;
  const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*GetClassData() const final;

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kQuestionerFieldNumber = 2,
    kQuestionFieldNumber = 3,
    kIdFieldNumber = 1,
  };
  // required string questioner = 2;
  bool has_questioner() const;
  private:
  bool _internal_has_questioner() const;
  public:
  void clear_questioner();
  const std::string& questioner() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_questioner(ArgT0&& arg0, ArgT... args);
  std::string* mutable_questioner();
  PROTOBUF_NODISCARD std::string* release_questioner();
  void set_allocated_questioner(std::string* questioner);
  private:
  const std::string& _internal_questioner() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_questioner(const std::string& value);
  std::string* _internal_mutable_questioner();
  public:

  // required string question = 3;
  bool has_question() const;
  private:
  bool _internal_has_question() const;
  public:
  void clear_question();
  const std::string& question() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_question(ArgT0&& arg0, ArgT... args);
  std::string* mutable_question();
  PROTOBUF_NODISCARD std::string* release_question();
  void set_allocated_question(std::string* question);
  private:
  const std::string& _internal_question() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_question(const std::string& value);
  std::string* _internal_mutable_question();
  public:

  // required int64 id = 1;
  bool has_id() const;
  private:
  bool _internal_has_id() const;
  public:
  void clear_id();
  int64_t id() const;
  void set_id(int64_t value);
  private:
  int64_t _internal_id() const;
  void _internal_set_id(int64_t value);
  public:

  // @@protoc_insertion_point(class_scope:test.Query)
 private:
  class _Internal;

  // helper for ByteSizeLong()
  size_t RequiredFieldsByteSizeFallback() const;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  struct Impl_ {
    ::PROTOBUF_NAMESPACE_ID::internal::HasBits<1> _has_bits_;
    mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
    ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr questioner_;
    ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr question_;
    int64_t id_;
  };
  union { Impl_ _impl_; };
  friend struct ::TableStruct_query_2eproto;
};
// -------------------------------------------------------------------

class Answer final :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:test.Answer) */ {
 public:
  inline Answer() : Answer(nullptr) {}
  ~Answer() override;
  explicit PROTOBUF_CONSTEXPR Answer(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  Answer(const Answer& from);
  Answer(Answer&& from) noexcept
    : Answer() {
    *this = ::std::move(from);
  }

  inline Answer& operator=(const Answer& from) {
    CopyFrom(from);
    return *this;
  }
  inline Answer& operator=(Answer&& from) noexcept {
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

  inline const ::PROTOBUF_NAMESPACE_ID::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance);
  }
  inline ::PROTOBUF_NAMESPACE_ID::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
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
  static const Answer& default_instance() {
    return *internal_default_instance();
  }
  static inline const Answer* internal_default_instance() {
    return reinterpret_cast<const Answer*>(
               &_Answer_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    1;

  friend void swap(Answer& a, Answer& b) {
    a.Swap(&b);
  }
  inline void Swap(Answer* other) {
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
  void UnsafeArenaSwap(Answer* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  Answer* New(::PROTOBUF_NAMESPACE_ID::Arena* arena = nullptr) const final {
    return CreateMaybeMessage<Answer>(arena);
  }
  using ::PROTOBUF_NAMESPACE_ID::Message::CopyFrom;
  void CopyFrom(const Answer& from);
  using ::PROTOBUF_NAMESPACE_ID::Message::MergeFrom;
  void MergeFrom( const Answer& from) {
    Answer::MergeImpl(*this, from);
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
  void InternalSwap(Answer* other);

  private:
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "test.Answer";
  }
  protected:
  explicit Answer(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  public:

  static const ClassData _class_data_;
  const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*GetClassData() const final;

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kSolutionFieldNumber = 4,
    kQuestionerFieldNumber = 2,
    kAnswererFieldNumber = 3,
    kIdFieldNumber = 1,
  };
  // repeated string solution = 4;
  int solution_size() const;
  private:
  int _internal_solution_size() const;
  public:
  void clear_solution();
  const std::string& solution(int index) const;
  std::string* mutable_solution(int index);
  void set_solution(int index, const std::string& value);
  void set_solution(int index, std::string&& value);
  void set_solution(int index, const char* value);
  void set_solution(int index, const char* value, size_t size);
  std::string* add_solution();
  void add_solution(const std::string& value);
  void add_solution(std::string&& value);
  void add_solution(const char* value);
  void add_solution(const char* value, size_t size);
  const ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>& solution() const;
  ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>* mutable_solution();
  private:
  const std::string& _internal_solution(int index) const;
  std::string* _internal_add_solution();
  public:

  // required string questioner = 2;
  bool has_questioner() const;
  private:
  bool _internal_has_questioner() const;
  public:
  void clear_questioner();
  const std::string& questioner() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_questioner(ArgT0&& arg0, ArgT... args);
  std::string* mutable_questioner();
  PROTOBUF_NODISCARD std::string* release_questioner();
  void set_allocated_questioner(std::string* questioner);
  private:
  const std::string& _internal_questioner() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_questioner(const std::string& value);
  std::string* _internal_mutable_questioner();
  public:

  // required string answerer = 3;
  bool has_answerer() const;
  private:
  bool _internal_has_answerer() const;
  public:
  void clear_answerer();
  const std::string& answerer() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_answerer(ArgT0&& arg0, ArgT... args);
  std::string* mutable_answerer();
  PROTOBUF_NODISCARD std::string* release_answerer();
  void set_allocated_answerer(std::string* answerer);
  private:
  const std::string& _internal_answerer() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_answerer(const std::string& value);
  std::string* _internal_mutable_answerer();
  public:

  // required int64 id = 1;
  bool has_id() const;
  private:
  bool _internal_has_id() const;
  public:
  void clear_id();
  int64_t id() const;
  void set_id(int64_t value);
  private:
  int64_t _internal_id() const;
  void _internal_set_id(int64_t value);
  public:

  // @@protoc_insertion_point(class_scope:test.Answer)
 private:
  class _Internal;

  // helper for ByteSizeLong()
  size_t RequiredFieldsByteSizeFallback() const;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  struct Impl_ {
    ::PROTOBUF_NAMESPACE_ID::internal::HasBits<1> _has_bits_;
    mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
    ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string> solution_;
    ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr questioner_;
    ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr answerer_;
    int64_t id_;
  };
  union { Impl_ _impl_; };
  friend struct ::TableStruct_query_2eproto;
};
// -------------------------------------------------------------------

class Empty final :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:test.Empty) */ {
 public:
  inline Empty() : Empty(nullptr) {}
  ~Empty() override;
  explicit PROTOBUF_CONSTEXPR Empty(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  Empty(const Empty& from);
  Empty(Empty&& from) noexcept
    : Empty() {
    *this = ::std::move(from);
  }

  inline Empty& operator=(const Empty& from) {
    CopyFrom(from);
    return *this;
  }
  inline Empty& operator=(Empty&& from) noexcept {
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

  inline const ::PROTOBUF_NAMESPACE_ID::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance);
  }
  inline ::PROTOBUF_NAMESPACE_ID::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
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
  static const Empty& default_instance() {
    return *internal_default_instance();
  }
  static inline const Empty* internal_default_instance() {
    return reinterpret_cast<const Empty*>(
               &_Empty_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    2;

  friend void swap(Empty& a, Empty& b) {
    a.Swap(&b);
  }
  inline void Swap(Empty* other) {
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
  void UnsafeArenaSwap(Empty* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  Empty* New(::PROTOBUF_NAMESPACE_ID::Arena* arena = nullptr) const final {
    return CreateMaybeMessage<Empty>(arena);
  }
  using ::PROTOBUF_NAMESPACE_ID::Message::CopyFrom;
  void CopyFrom(const Empty& from);
  using ::PROTOBUF_NAMESPACE_ID::Message::MergeFrom;
  void MergeFrom( const Empty& from) {
    Empty::MergeImpl(*this, from);
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
  void InternalSwap(Empty* other);

  private:
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "test.Empty";
  }
  protected:
  explicit Empty(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  public:

  static const ClassData _class_data_;
  const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*GetClassData() const final;

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kIdFieldNumber = 1,
  };
  // optional int32 id = 1;
  bool has_id() const;
  private:
  bool _internal_has_id() const;
  public:
  void clear_id();
  int32_t id() const;
  void set_id(int32_t value);
  private:
  int32_t _internal_id() const;
  void _internal_set_id(int32_t value);
  public:

  // @@protoc_insertion_point(class_scope:test.Empty)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  struct Impl_ {
    ::PROTOBUF_NAMESPACE_ID::internal::HasBits<1> _has_bits_;
    mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
    int32_t id_;
  };
  union { Impl_ _impl_; };
  friend struct ::TableStruct_query_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// Query

// required int64 id = 1;
inline bool Query::_internal_has_id() const {
  bool value = (_impl_._has_bits_[0] & 0x00000004u) != 0;
  return value;
}
inline bool Query::has_id() const {
  return _internal_has_id();
}
inline void Query::clear_id() {
  _impl_.id_ = int64_t{0};
  _impl_._has_bits_[0] &= ~0x00000004u;
}
inline int64_t Query::_internal_id() const {
  return _impl_.id_;
}
inline int64_t Query::id() const {
  // @@protoc_insertion_point(field_get:test.Query.id)
  return _internal_id();
}
inline void Query::_internal_set_id(int64_t value) {
  _impl_._has_bits_[0] |= 0x00000004u;
  _impl_.id_ = value;
}
inline void Query::set_id(int64_t value) {
  _internal_set_id(value);
  // @@protoc_insertion_point(field_set:test.Query.id)
}

// required string questioner = 2;
inline bool Query::_internal_has_questioner() const {
  bool value = (_impl_._has_bits_[0] & 0x00000001u) != 0;
  return value;
}
inline bool Query::has_questioner() const {
  return _internal_has_questioner();
}
inline void Query::clear_questioner() {
  _impl_.questioner_.ClearToEmpty();
  _impl_._has_bits_[0] &= ~0x00000001u;
}
inline const std::string& Query::questioner() const {
  // @@protoc_insertion_point(field_get:test.Query.questioner)
  return _internal_questioner();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void Query::set_questioner(ArgT0&& arg0, ArgT... args) {
 _impl_._has_bits_[0] |= 0x00000001u;
 _impl_.questioner_.Set(static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:test.Query.questioner)
}
inline std::string* Query::mutable_questioner() {
  std::string* _s = _internal_mutable_questioner();
  // @@protoc_insertion_point(field_mutable:test.Query.questioner)
  return _s;
}
inline const std::string& Query::_internal_questioner() const {
  return _impl_.questioner_.Get();
}
inline void Query::_internal_set_questioner(const std::string& value) {
  _impl_._has_bits_[0] |= 0x00000001u;
  _impl_.questioner_.Set(value, GetArenaForAllocation());
}
inline std::string* Query::_internal_mutable_questioner() {
  _impl_._has_bits_[0] |= 0x00000001u;
  return _impl_.questioner_.Mutable(GetArenaForAllocation());
}
inline std::string* Query::release_questioner() {
  // @@protoc_insertion_point(field_release:test.Query.questioner)
  if (!_internal_has_questioner()) {
    return nullptr;
  }
  _impl_._has_bits_[0] &= ~0x00000001u;
  auto* p = _impl_.questioner_.Release();
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (_impl_.questioner_.IsDefault()) {
    _impl_.questioner_.Set("", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  return p;
}
inline void Query::set_allocated_questioner(std::string* questioner) {
  if (questioner != nullptr) {
    _impl_._has_bits_[0] |= 0x00000001u;
  } else {
    _impl_._has_bits_[0] &= ~0x00000001u;
  }
  _impl_.questioner_.SetAllocated(questioner, GetArenaForAllocation());
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (_impl_.questioner_.IsDefault()) {
    _impl_.questioner_.Set("", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  // @@protoc_insertion_point(field_set_allocated:test.Query.questioner)
}

// required string question = 3;
inline bool Query::_internal_has_question() const {
  bool value = (_impl_._has_bits_[0] & 0x00000002u) != 0;
  return value;
}
inline bool Query::has_question() const {
  return _internal_has_question();
}
inline void Query::clear_question() {
  _impl_.question_.ClearToEmpty();
  _impl_._has_bits_[0] &= ~0x00000002u;
}
inline const std::string& Query::question() const {
  // @@protoc_insertion_point(field_get:test.Query.question)
  return _internal_question();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void Query::set_question(ArgT0&& arg0, ArgT... args) {
 _impl_._has_bits_[0] |= 0x00000002u;
 _impl_.question_.Set(static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:test.Query.question)
}
inline std::string* Query::mutable_question() {
  std::string* _s = _internal_mutable_question();
  // @@protoc_insertion_point(field_mutable:test.Query.question)
  return _s;
}
inline const std::string& Query::_internal_question() const {
  return _impl_.question_.Get();
}
inline void Query::_internal_set_question(const std::string& value) {
  _impl_._has_bits_[0] |= 0x00000002u;
  _impl_.question_.Set(value, GetArenaForAllocation());
}
inline std::string* Query::_internal_mutable_question() {
  _impl_._has_bits_[0] |= 0x00000002u;
  return _impl_.question_.Mutable(GetArenaForAllocation());
}
inline std::string* Query::release_question() {
  // @@protoc_insertion_point(field_release:test.Query.question)
  if (!_internal_has_question()) {
    return nullptr;
  }
  _impl_._has_bits_[0] &= ~0x00000002u;
  auto* p = _impl_.question_.Release();
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (_impl_.question_.IsDefault()) {
    _impl_.question_.Set("", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  return p;
}
inline void Query::set_allocated_question(std::string* question) {
  if (question != nullptr) {
    _impl_._has_bits_[0] |= 0x00000002u;
  } else {
    _impl_._has_bits_[0] &= ~0x00000002u;
  }
  _impl_.question_.SetAllocated(question, GetArenaForAllocation());
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (_impl_.question_.IsDefault()) {
    _impl_.question_.Set("", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  // @@protoc_insertion_point(field_set_allocated:test.Query.question)
}

// -------------------------------------------------------------------

// Answer

// required int64 id = 1;
inline bool Answer::_internal_has_id() const {
  bool value = (_impl_._has_bits_[0] & 0x00000004u) != 0;
  return value;
}
inline bool Answer::has_id() const {
  return _internal_has_id();
}
inline void Answer::clear_id() {
  _impl_.id_ = int64_t{0};
  _impl_._has_bits_[0] &= ~0x00000004u;
}
inline int64_t Answer::_internal_id() const {
  return _impl_.id_;
}
inline int64_t Answer::id() const {
  // @@protoc_insertion_point(field_get:test.Answer.id)
  return _internal_id();
}
inline void Answer::_internal_set_id(int64_t value) {
  _impl_._has_bits_[0] |= 0x00000004u;
  _impl_.id_ = value;
}
inline void Answer::set_id(int64_t value) {
  _internal_set_id(value);
  // @@protoc_insertion_point(field_set:test.Answer.id)
}

// required string questioner = 2;
inline bool Answer::_internal_has_questioner() const {
  bool value = (_impl_._has_bits_[0] & 0x00000001u) != 0;
  return value;
}
inline bool Answer::has_questioner() const {
  return _internal_has_questioner();
}
inline void Answer::clear_questioner() {
  _impl_.questioner_.ClearToEmpty();
  _impl_._has_bits_[0] &= ~0x00000001u;
}
inline const std::string& Answer::questioner() const {
  // @@protoc_insertion_point(field_get:test.Answer.questioner)
  return _internal_questioner();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void Answer::set_questioner(ArgT0&& arg0, ArgT... args) {
 _impl_._has_bits_[0] |= 0x00000001u;
 _impl_.questioner_.Set(static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:test.Answer.questioner)
}
inline std::string* Answer::mutable_questioner() {
  std::string* _s = _internal_mutable_questioner();
  // @@protoc_insertion_point(field_mutable:test.Answer.questioner)
  return _s;
}
inline const std::string& Answer::_internal_questioner() const {
  return _impl_.questioner_.Get();
}
inline void Answer::_internal_set_questioner(const std::string& value) {
  _impl_._has_bits_[0] |= 0x00000001u;
  _impl_.questioner_.Set(value, GetArenaForAllocation());
}
inline std::string* Answer::_internal_mutable_questioner() {
  _impl_._has_bits_[0] |= 0x00000001u;
  return _impl_.questioner_.Mutable(GetArenaForAllocation());
}
inline std::string* Answer::release_questioner() {
  // @@protoc_insertion_point(field_release:test.Answer.questioner)
  if (!_internal_has_questioner()) {
    return nullptr;
  }
  _impl_._has_bits_[0] &= ~0x00000001u;
  auto* p = _impl_.questioner_.Release();
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (_impl_.questioner_.IsDefault()) {
    _impl_.questioner_.Set("", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  return p;
}
inline void Answer::set_allocated_questioner(std::string* questioner) {
  if (questioner != nullptr) {
    _impl_._has_bits_[0] |= 0x00000001u;
  } else {
    _impl_._has_bits_[0] &= ~0x00000001u;
  }
  _impl_.questioner_.SetAllocated(questioner, GetArenaForAllocation());
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (_impl_.questioner_.IsDefault()) {
    _impl_.questioner_.Set("", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  // @@protoc_insertion_point(field_set_allocated:test.Answer.questioner)
}

// required string answerer = 3;
inline bool Answer::_internal_has_answerer() const {
  bool value = (_impl_._has_bits_[0] & 0x00000002u) != 0;
  return value;
}
inline bool Answer::has_answerer() const {
  return _internal_has_answerer();
}
inline void Answer::clear_answerer() {
  _impl_.answerer_.ClearToEmpty();
  _impl_._has_bits_[0] &= ~0x00000002u;
}
inline const std::string& Answer::answerer() const {
  // @@protoc_insertion_point(field_get:test.Answer.answerer)
  return _internal_answerer();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void Answer::set_answerer(ArgT0&& arg0, ArgT... args) {
 _impl_._has_bits_[0] |= 0x00000002u;
 _impl_.answerer_.Set(static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:test.Answer.answerer)
}
inline std::string* Answer::mutable_answerer() {
  std::string* _s = _internal_mutable_answerer();
  // @@protoc_insertion_point(field_mutable:test.Answer.answerer)
  return _s;
}
inline const std::string& Answer::_internal_answerer() const {
  return _impl_.answerer_.Get();
}
inline void Answer::_internal_set_answerer(const std::string& value) {
  _impl_._has_bits_[0] |= 0x00000002u;
  _impl_.answerer_.Set(value, GetArenaForAllocation());
}
inline std::string* Answer::_internal_mutable_answerer() {
  _impl_._has_bits_[0] |= 0x00000002u;
  return _impl_.answerer_.Mutable(GetArenaForAllocation());
}
inline std::string* Answer::release_answerer() {
  // @@protoc_insertion_point(field_release:test.Answer.answerer)
  if (!_internal_has_answerer()) {
    return nullptr;
  }
  _impl_._has_bits_[0] &= ~0x00000002u;
  auto* p = _impl_.answerer_.Release();
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (_impl_.answerer_.IsDefault()) {
    _impl_.answerer_.Set("", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  return p;
}
inline void Answer::set_allocated_answerer(std::string* answerer) {
  if (answerer != nullptr) {
    _impl_._has_bits_[0] |= 0x00000002u;
  } else {
    _impl_._has_bits_[0] &= ~0x00000002u;
  }
  _impl_.answerer_.SetAllocated(answerer, GetArenaForAllocation());
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (_impl_.answerer_.IsDefault()) {
    _impl_.answerer_.Set("", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  // @@protoc_insertion_point(field_set_allocated:test.Answer.answerer)
}

// repeated string solution = 4;
inline int Answer::_internal_solution_size() const {
  return _impl_.solution_.size();
}
inline int Answer::solution_size() const {
  return _internal_solution_size();
}
inline void Answer::clear_solution() {
  _impl_.solution_.Clear();
}
inline std::string* Answer::add_solution() {
  std::string* _s = _internal_add_solution();
  // @@protoc_insertion_point(field_add_mutable:test.Answer.solution)
  return _s;
}
inline const std::string& Answer::_internal_solution(int index) const {
  return _impl_.solution_.Get(index);
}
inline const std::string& Answer::solution(int index) const {
  // @@protoc_insertion_point(field_get:test.Answer.solution)
  return _internal_solution(index);
}
inline std::string* Answer::mutable_solution(int index) {
  // @@protoc_insertion_point(field_mutable:test.Answer.solution)
  return _impl_.solution_.Mutable(index);
}
inline void Answer::set_solution(int index, const std::string& value) {
  _impl_.solution_.Mutable(index)->assign(value);
  // @@protoc_insertion_point(field_set:test.Answer.solution)
}
inline void Answer::set_solution(int index, std::string&& value) {
  _impl_.solution_.Mutable(index)->assign(std::move(value));
  // @@protoc_insertion_point(field_set:test.Answer.solution)
}
inline void Answer::set_solution(int index, const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  _impl_.solution_.Mutable(index)->assign(value);
  // @@protoc_insertion_point(field_set_char:test.Answer.solution)
}
inline void Answer::set_solution(int index, const char* value, size_t size) {
  _impl_.solution_.Mutable(index)->assign(
    reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:test.Answer.solution)
}
inline std::string* Answer::_internal_add_solution() {
  return _impl_.solution_.Add();
}
inline void Answer::add_solution(const std::string& value) {
  _impl_.solution_.Add()->assign(value);
  // @@protoc_insertion_point(field_add:test.Answer.solution)
}
inline void Answer::add_solution(std::string&& value) {
  _impl_.solution_.Add(std::move(value));
  // @@protoc_insertion_point(field_add:test.Answer.solution)
}
inline void Answer::add_solution(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  _impl_.solution_.Add()->assign(value);
  // @@protoc_insertion_point(field_add_char:test.Answer.solution)
}
inline void Answer::add_solution(const char* value, size_t size) {
  _impl_.solution_.Add()->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_add_pointer:test.Answer.solution)
}
inline const ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>&
Answer::solution() const {
  // @@protoc_insertion_point(field_list:test.Answer.solution)
  return _impl_.solution_;
}
inline ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>*
Answer::mutable_solution() {
  // @@protoc_insertion_point(field_mutable_list:test.Answer.solution)
  return &_impl_.solution_;
}

// -------------------------------------------------------------------

// Empty

// optional int32 id = 1;
inline bool Empty::_internal_has_id() const {
  bool value = (_impl_._has_bits_[0] & 0x00000001u) != 0;
  return value;
}
inline bool Empty::has_id() const {
  return _internal_has_id();
}
inline void Empty::clear_id() {
  _impl_.id_ = 0;
  _impl_._has_bits_[0] &= ~0x00000001u;
}
inline int32_t Empty::_internal_id() const {
  return _impl_.id_;
}
inline int32_t Empty::id() const {
  // @@protoc_insertion_point(field_get:test.Empty.id)
  return _internal_id();
}
inline void Empty::_internal_set_id(int32_t value) {
  _impl_._has_bits_[0] |= 0x00000001u;
  _impl_.id_ = value;
}
inline void Empty::set_id(int32_t value) {
  _internal_set_id(value);
  // @@protoc_insertion_point(field_set:test.Empty.id)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__
// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace test

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_query_2eproto
