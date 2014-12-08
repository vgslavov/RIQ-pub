// Generated by the protocol buffer compiler.  DO NOT EDIT!

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "ris.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)

namespace ris {

namespace {

const ::google::protobuf::Descriptor* HashValue_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  HashValue_reflection_ = NULL;
const ::google::protobuf::Descriptor* LSHindex_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  LSHindex_reflection_ = NULL;

}  // namespace


void protobuf_AssignDesc_ris_2eproto() {
  protobuf_AddDesc_ris_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "ris.proto");
  GOOGLE_CHECK(file != NULL);
  HashValue_descriptor_ = file->message_type(0);
  static const int HashValue_offsets_[2] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(HashValue, hash_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(HashValue, freq_),
  };
  HashValue_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      HashValue_descriptor_,
      HashValue::default_instance_,
      HashValue_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(HashValue, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(HashValue, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(HashValue));
  LSHindex_descriptor_ = file->message_type(1);
  static const int LSHindex_offsets_[1] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(LSHindex, hashvalue_),
  };
  LSHindex_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      LSHindex_descriptor_,
      LSHindex::default_instance_,
      LSHindex_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(LSHindex, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(LSHindex, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(LSHindex));
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_ris_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    HashValue_descriptor_, &HashValue::default_instance());
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    LSHindex_descriptor_, &LSHindex::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_ris_2eproto() {
  delete HashValue::default_instance_;
  delete HashValue_reflection_;
  delete LSHindex::default_instance_;
  delete LSHindex_reflection_;
}

void protobuf_AddDesc_ris_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\tris.proto\022\003ris\"\'\n\tHashValue\022\014\n\004hash\030\001 "
    "\002(\t\022\014\n\004freq\030\002 \002(\005\"-\n\010LSHindex\022!\n\thashval"
    "ue\030\001 \003(\0132\016.ris.HashValue", 104);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "ris.proto", &protobuf_RegisterTypes);
  HashValue::default_instance_ = new HashValue();
  LSHindex::default_instance_ = new LSHindex();
  HashValue::default_instance_->InitAsDefaultInstance();
  LSHindex::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_ris_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_ris_2eproto {
  StaticDescriptorInitializer_ris_2eproto() {
    protobuf_AddDesc_ris_2eproto();
  }
} static_descriptor_initializer_ris_2eproto_;


// ===================================================================

#ifndef _MSC_VER
const int HashValue::kHashFieldNumber;
const int HashValue::kFreqFieldNumber;
#endif  // !_MSC_VER

HashValue::HashValue()
  : ::google::protobuf::Message() {
  SharedCtor();
}

void HashValue::InitAsDefaultInstance() {
}

HashValue::HashValue(const HashValue& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
}

void HashValue::SharedCtor() {
  _cached_size_ = 0;
  hash_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  freq_ = 0;
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

HashValue::~HashValue() {
  SharedDtor();
}

void HashValue::SharedDtor() {
  if (hash_ != &::google::protobuf::internal::kEmptyString) {
    delete hash_;
  }
  if (this != default_instance_) {
  }
}

void HashValue::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* HashValue::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return HashValue_descriptor_;
}

const HashValue& HashValue::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_ris_2eproto();  return *default_instance_;
}

HashValue* HashValue::default_instance_ = NULL;

HashValue* HashValue::New() const {
  return new HashValue;
}

void HashValue::Clear() {
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (has_hash()) {
      if (hash_ != &::google::protobuf::internal::kEmptyString) {
        hash_->clear();
      }
    }
    freq_ = 0;
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool HashValue::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) return false
  ::google::protobuf::uint32 tag;
  while ((tag = input->ReadTag()) != 0) {
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required string hash = 1;
      case 1: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_hash()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->hash().data(), this->hash().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(16)) goto parse_freq;
        break;
      }
      
      // required int32 freq = 2;
      case 2: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_freq:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &freq_)));
          set_has_freq();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectAtEnd()) return true;
        break;
      }
      
      default: {
      handle_uninterpreted:
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          return true;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, mutable_unknown_fields()));
        break;
      }
    }
  }
  return true;
#undef DO_
}

void HashValue::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // required string hash = 1;
  if (has_hash()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->hash().data(), this->hash().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      1, this->hash(), output);
  }
  
  // required int32 freq = 2;
  if (has_freq()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(2, this->freq(), output);
  }
  
  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
}

::google::protobuf::uint8* HashValue::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // required string hash = 1;
  if (has_hash()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->hash().data(), this->hash().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        1, this->hash(), target);
  }
  
  // required int32 freq = 2;
  if (has_freq()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(2, this->freq(), target);
  }
  
  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  return target;
}

int HashValue::ByteSize() const {
  int total_size = 0;
  
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // required string hash = 1;
    if (has_hash()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->hash());
    }
    
    // required int32 freq = 2;
    if (has_freq()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->freq());
    }
    
  }
  if (!unknown_fields().empty()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void HashValue::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const HashValue* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const HashValue*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void HashValue::MergeFrom(const HashValue& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_hash()) {
      set_hash(from.hash());
    }
    if (from.has_freq()) {
      set_freq(from.freq());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void HashValue::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void HashValue::CopyFrom(const HashValue& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool HashValue::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000003) != 0x00000003) return false;
  
  return true;
}

void HashValue::Swap(HashValue* other) {
  if (other != this) {
    std::swap(hash_, other->hash_);
    std::swap(freq_, other->freq_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata HashValue::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = HashValue_descriptor_;
  metadata.reflection = HashValue_reflection_;
  return metadata;
}


// ===================================================================

#ifndef _MSC_VER
const int LSHindex::kHashvalueFieldNumber;
#endif  // !_MSC_VER

LSHindex::LSHindex()
  : ::google::protobuf::Message() {
  SharedCtor();
}

void LSHindex::InitAsDefaultInstance() {
}

LSHindex::LSHindex(const LSHindex& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
}

void LSHindex::SharedCtor() {
  _cached_size_ = 0;
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

LSHindex::~LSHindex() {
  SharedDtor();
}

void LSHindex::SharedDtor() {
  if (this != default_instance_) {
  }
}

void LSHindex::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* LSHindex::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return LSHindex_descriptor_;
}

const LSHindex& LSHindex::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_ris_2eproto();  return *default_instance_;
}

LSHindex* LSHindex::default_instance_ = NULL;

LSHindex* LSHindex::New() const {
  return new LSHindex;
}

void LSHindex::Clear() {
  hashvalue_.Clear();
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool LSHindex::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) return false
  ::google::protobuf::uint32 tag;
  while ((tag = input->ReadTag()) != 0) {
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // repeated .ris.HashValue hashvalue = 1;
      case 1: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_hashvalue:
          DO_(::google::protobuf::internal::WireFormatLite::ReadMessageNoVirtual(
                input, add_hashvalue()));
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(10)) goto parse_hashvalue;
        if (input->ExpectAtEnd()) return true;
        break;
      }
      
      default: {
      handle_uninterpreted:
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          return true;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, mutable_unknown_fields()));
        break;
      }
    }
  }
  return true;
#undef DO_
}

void LSHindex::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // repeated .ris.HashValue hashvalue = 1;
  for (int i = 0; i < this->hashvalue_size(); i++) {
    ::google::protobuf::internal::WireFormatLite::WriteMessageMaybeToArray(
      1, this->hashvalue(i), output);
  }
  
  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
}

::google::protobuf::uint8* LSHindex::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // repeated .ris.HashValue hashvalue = 1;
  for (int i = 0; i < this->hashvalue_size(); i++) {
    target = ::google::protobuf::internal::WireFormatLite::
      WriteMessageNoVirtualToArray(
        1, this->hashvalue(i), target);
  }
  
  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  return target;
}

int LSHindex::ByteSize() const {
  int total_size = 0;
  
  // repeated .ris.HashValue hashvalue = 1;
  total_size += 1 * this->hashvalue_size();
  for (int i = 0; i < this->hashvalue_size(); i++) {
    total_size +=
      ::google::protobuf::internal::WireFormatLite::MessageSizeNoVirtual(
        this->hashvalue(i));
  }
  
  if (!unknown_fields().empty()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void LSHindex::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const LSHindex* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const LSHindex*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void LSHindex::MergeFrom(const LSHindex& from) {
  GOOGLE_CHECK_NE(&from, this);
  hashvalue_.MergeFrom(from.hashvalue_);
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void LSHindex::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void LSHindex::CopyFrom(const LSHindex& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool LSHindex::IsInitialized() const {
  
  for (int i = 0; i < hashvalue_size(); i++) {
    if (!this->hashvalue(i).IsInitialized()) return false;
  }
  return true;
}

void LSHindex::Swap(LSHindex* other) {
  if (other != this) {
    hashvalue_.Swap(&other->hashvalue_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata LSHindex::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = LSHindex_descriptor_;
  metadata.reflection = LSHindex_reflection_;
  return metadata;
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace ris

// @@protoc_insertion_point(global_scope)