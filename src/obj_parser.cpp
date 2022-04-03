/*
 * obj_parser.cc
 * Copyright (C) 2020  Robin Verschoren
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "obj_parser.h"

#include <algorithm>
#include <cassert>
#include <exception>
#include <functional>
#include <iomanip>
#include <iterator>
#include <memory>
#include <sstream>

namespace obj {

ParseException::ParseException() throw() : std::exception()
{
  // Does nothing...
}

ParseException::ParseException(const ParseException& original) throw() : std::exception(original)
{
  // Does nothing...
}

ParseException::~ParseException() throw()
{
  // Does nothing...
}

ParseException& ParseException::operator=(const ParseException& original) throw()
{
  std::exception::operator=(original);
  return *this;
}

UnexpectedCharacter::UnexpectedCharacter(const std::istream::int_type character_init,
                                         const std::istream::pos_type position_init) throw()
    : ParseException(), character(character_init), position(position_init), message("")
{
  std::ostringstream stream;

  if (character == std::istream::traits_type::eof()) {
    stream << "end-of-file";
  } else {
    stream << "character \'";

    if (std::isprint(character)) {
      stream << static_cast<char>(character);
    } else {
      stream << "\\" << std::oct << std::setfill('0') << std::setw(3) << static_cast<int>(character);
    }

    stream << "\'";
  }

  stream << " not expected at position " << position;
  message = stream.str();
}

UnexpectedCharacter::UnexpectedCharacter(const UnexpectedCharacter& original) throw()
    : ParseException(), character(original.character), position(original.position), message(original.message)
{
  // Does nothing...
}

UnexpectedCharacter::~UnexpectedCharacter() throw()
{
  // Does nothing...
}

UnexpectedCharacter& UnexpectedCharacter::operator=(const UnexpectedCharacter& original) throw()
{
  std::exception::operator=(original);

  character = original.character;
  position = original.position;
  message = original.message;

  return *this;
}

const char* UnexpectedCharacter::what() const throw() { return message.c_str(); }

DuplicateMaterial::DuplicateMaterial(const std::string& name_init) throw()
    : ParseException(), name(name_init), message("")
{
  message = "material named \'" + name + "\' ecountered twice";
}

DuplicateMaterial::DuplicateMaterial(const DuplicateMaterial& original) throw()
    : ParseException(), name(original.name), message(original.message)
{
  // Does nothing...
}

DuplicateMaterial::~DuplicateMaterial() throw()
{
  // Does nothing...
}

DuplicateMaterial& DuplicateMaterial::operator=(const DuplicateMaterial& original) throw()
{
  std::exception::operator=(original);

  name = original.name;
  message = original.message;

  return *this;
}

const char* DuplicateMaterial::what() const throw() { return message.c_str(); }

MalformedComment::MalformedComment(const std::string& line_init) throw()
    : ParseException(), line(line_init), message("")
{
  message = "malformed comment encountered in line \'" + line + "\'";
}

MalformedComment::MalformedComment(const MalformedComment& original) throw()
    : ParseException(), line(original.line), message(original.message)
{
  // Does nothing...
}

MalformedComment::~MalformedComment() throw()
{
  // Does nothing...
}

MalformedComment& MalformedComment::operator=(const MalformedComment& original) throw()
{
  std::exception::operator=(original);
  message = original.message;
  return *this;
}

const char* MalformedComment::what() const throw() { return message.c_str(); }

MalformedVertex::MalformedVertex(const std::string& line_init) throw() : ParseException(), line(line_init), message("")
{
  message = "malformed coordinates specification \'" + line + "\' encountered";
}

MalformedVertex::MalformedVertex(const MalformedVertex& original) throw()
    : ParseException(), line(original.line), message(original.message)
{
  // Does nothing...
}

MalformedVertex::~MalformedVertex() throw()
{
  // Does nothing...
}

MalformedVertex& MalformedVertex::operator=(const MalformedVertex& original) throw()
{
  std::exception::operator=(original);

  line = original.line;
  message = original.message;

  return *this;
}

const char* MalformedVertex::what() const throw() { return message.c_str(); }

MalformedPolygon::MalformedPolygon(const std::string& line_init) throw()
    : ParseException(), line(line_init), message("")
{
  message = "malformed polygon specification \'" + line + "\' encountered";
}

MalformedPolygon::MalformedPolygon(const MalformedPolygon& original) throw()
    : ParseException(), line(original.line), message(original.message)
{
  // Does nothing...
}

MalformedPolygon::~MalformedPolygon() throw()
{
  // Does nothing...
}

MalformedPolygon& MalformedPolygon::operator=(const MalformedPolygon& original) throw()
{
  std::exception::operator=(original);

  line = original.line;
  message = original.message;

  return *this;
}

const char* MalformedPolygon::what() const throw() { return message.c_str(); }

UnrecognizedType::UnrecognizedType(const std::string& line_init) throw()
    : ParseException(), line(line_init), message("")
{
  message = "unrecognized type specification \'" + line + "\' encountered";
}

UnrecognizedType::UnrecognizedType(const UnrecognizedType& original) throw()
    : ParseException(), line(original.line), message(original.message)
{
  // Does nothing...
}

UnrecognizedType::~UnrecognizedType() throw()
{
  // Does nothing...
}

UnrecognizedType& UnrecognizedType::operator=(const UnrecognizedType& original) throw()
{
  std::exception::operator=(original);

  line = original.line;
  message = original.message;

  return *this;
}

const char* UnrecognizedType::what() const throw() { return message.c_str(); }

DuplicateLine::DuplicateLine(const std::string& line_init) throw() : ParseException(), line(line_init), message("")
{
  message = "duplicate line encountered for entry on line \'" + line + "\'";
}

DuplicateLine::DuplicateLine(const DuplicateLine& original) throw()
    : ParseException(), line(original.line), message(original.message)
{
  // Does nothing...
}

DuplicateLine::~DuplicateLine() throw()
{
  // Does nothing...
}

DuplicateLine& DuplicateLine::operator=(const DuplicateLine& original) throw()
{
  std::exception::operator=(original);

  line = original.line;
  message = original.message;

  return *this;
}

const char* DuplicateLine::what() const throw() { return message.c_str(); }

MalformedLine::MalformedLine(const std::string& name_init, const std::string& line_init) throw()
    : ParseException(), line(line_init), message("")
{
  message = "malformed line  for key \'" + name_init + "\' encountered in line \'" + line + "\'";
}

MalformedLine::MalformedLine(const MalformedLine& original) throw()
    : ParseException(), line(original.line), message(original.message)
{
  // Does nothing...
}

MalformedLine::~MalformedLine() throw()
{
  // Does nothing...
}

MalformedLine& MalformedLine::operator=(const MalformedLine& original) throw()
{
  std::exception::operator=(original);

  line = original.line;
  message = original.message;

  return *this;
}

const char* MalformedLine::what() const throw() { return message.c_str(); }

DuplicateEntry::DuplicateEntry(const std::string& section_init, const std::string& key_init) throw()
    : ParseException(), section(section_init), key(key_init), message("")
{
  message = "entry named \'" + key + "\' ecountered twice in section \'" + section + "\'";
}

DuplicateEntry::DuplicateEntry(const DuplicateEntry& original) throw()
    : ParseException(), section(original.section), key(original.key), message(original.message)
{
  // Does nothing...
}

DuplicateEntry::~DuplicateEntry() throw()
{
  // Does nothing...
}

DuplicateEntry& DuplicateEntry::operator=(const DuplicateEntry& original) throw()
{
  std::exception::operator=(original);

  section = original.section;
  key = original.key;
  message = original.message;

  return *this;
}

const char* DuplicateEntry::what() const throw() { return message.c_str(); }

NonexistentEntry::NonexistentEntry(const std::string& section_name_init, const std::string& entry_name_init) throw()
    : std::exception(), section_name(section_name_init), entry_name(entry_name_init), message("")
{
  message = "entry with name \'" + entry_name + "\' does not exist in section \'" + section_name + "\'";
}

NonexistentEntry::NonexistentEntry(const NonexistentEntry& original) throw()
    : std::exception(original), message(original.message)
{
  // Does nothing...
}

NonexistentEntry::~NonexistentEntry() throw()
{
  // Does nothing...
}

NonexistentEntry& NonexistentEntry::operator=(const NonexistentEntry& original) throw()
{
  std::exception::operator=(original);
  message = original.message;
  return *this;
}

const char* NonexistentEntry::what() const throw() { return message.c_str(); }

IncompatibleConversion::IncompatibleConversion(const std::string& section_name_init, const std::string& entry_name_init,
                                               const std::string& type_name_init) throw()
    : std::exception(), section_name(section_name_init), entry_name(entry_name_init), type_name(type_name_init),
      message("")
{
  message = "cannot convert value of \'" + entry_name + "\' in section \'" + section_name + "\' to " + type_name;
}

IncompatibleConversion::IncompatibleConversion(const IncompatibleConversion& original) throw()
    : std::exception(original), section_name(original.section_name), entry_name(original.entry_name),
      type_name(original.type_name), message(original.message)
{
  // Does nothing...
}

IncompatibleConversion::~IncompatibleConversion() throw()
{
  // Does nothing...
}

IncompatibleConversion& IncompatibleConversion::operator=(const IncompatibleConversion& original) throw()
{
  std::exception::operator=(original);

  section_name = original.section_name;
  entry_name = original.entry_name;
  type_name = original.type_name;
  message = original.message;

  return *this;
}

const char* IncompatibleConversion::what() const throw() { return message.c_str(); }

class MTLValue
{
  protected:
  MTLValue();

  public:
  virtual ~MTLValue();

  virtual bool exists() const = 0;

  virtual bool as_int_if_exists(const std::string& material_name, const std::string& entry_name,
                                int& ret_val) const = 0;
  virtual bool as_double_if_exists(const std::string& material_name, const std::string& entry_name,
                                   double& ret_val) const = 0;
  virtual bool as_string_if_exists(const std::string& material_name, const std::string& entry_name,
                                   std::string& ret_val) const = 0;
  virtual bool as_bool_if_exists(const std::string& material_name, const std::string& entry_name,
                                 bool& ret_val) const = 0;
  virtual bool as_int_tuple_if_exists(const std::string& material_name, const std::string& entry_name,
                                      IntTuple& ret_val) const = 0;
  virtual bool as_double_tuple_if_exists(const std::string& material_name, const std::string& entry_name,
                                         DoubleTuple& ret_val) const = 0;

  virtual void print(std::ostream& output_stream) const = 0;
};

MTLValue::MTLValue()
{
  // Does nothing...
}

MTLValue::~MTLValue()
{
  // Does nothing...
}

namespace {
class MTLValueBase : public MTLValue
{
  protected:
  MTLValueBase();

  public:
  virtual ~MTLValueBase();

  virtual bool exists() const;

  virtual bool as_int_if_exists(const std::string& section_name, const std::string& entry_name, int& ret_val) const;
  virtual bool as_double_if_exists(const std::string& section_name, const std::string& entry_name,
                                   double& ret_val) const;
  virtual bool as_string_if_exists(const std::string& section_name, const std::string& entry_name,
                                   std::string& ret_val) const;
  virtual bool as_bool_if_exists(const std::string& section_name, const std::string& entry_name, bool& ret_val) const;
  virtual bool as_int_tuple_if_exists(const std::string& section_name, const std::string& entry_name,
                                      IntTuple& ret_val) const;
  virtual bool as_double_tuple_if_exists(const std::string& section_name, const std::string& entry_name,
                                         DoubleTuple& ret_val) const;
};

MTLValueBase::MTLValueBase()
{
  // Does nothing...
}

MTLValueBase::~MTLValueBase()
{
  // Does nothing...
}

bool MTLValueBase::exists() const { return true; }

bool MTLValueBase::as_int_if_exists(const std::string& section_name, const std::string& entry_name,
                                    int& /*ret_val*/) const
{
  throw IncompatibleConversion(section_name, entry_name, "int");
}

bool MTLValueBase::as_double_if_exists(const std::string& section_name, const std::string& entry_name,
                                       double& /*ret_val*/) const
{
  throw IncompatibleConversion(section_name, entry_name, "double");
}

bool MTLValueBase::as_string_if_exists(const std::string& section_name, const std::string& entry_name,
                                       std::string& /*ret_val*/) const
{
  throw IncompatibleConversion(section_name, entry_name, "string");
}

bool MTLValueBase::as_bool_if_exists(const std::string& section_name, const std::string& entry_name,
                                     bool& /*ret_val*/) const
{
  throw IncompatibleConversion(section_name, entry_name, "bool");
}

bool MTLValueBase::as_int_tuple_if_exists(const std::string& section_name, const std::string& entry_name,
                                          IntTuple& /*ret_val*/) const
{
  throw IncompatibleConversion(section_name, entry_name, "int tuple");
}

bool MTLValueBase::as_double_tuple_if_exists(const std::string& section_name, const std::string& entry_name,
                                             DoubleTuple& /*ret_val*/) const
{
  throw IncompatibleConversion(section_name, entry_name, "double tuple");
}

class IntValue : public MTLValueBase
{
  private:
  const int value;

  public:
  IntValue(const int value_init);
  virtual ~IntValue();

  virtual bool as_int_if_exists(const std::string& section_name, const std::string& entry_name, int& ret_val) const;
  virtual bool as_double_if_exists(const std::string& section_name, const std::string& entry_name,
                                   double& ret_val) const;

  virtual void print(std::ostream& output_stream) const;
};

IntValue::IntValue(const int value_init) : MTLValueBase(), value(value_init)
{
  // Does nothing...
}

IntValue::~IntValue()
{
  // Does nothing...
}

bool IntValue::as_int_if_exists(const std::string& /*section_name*/, const std::string& /*entry_name*/,
                                int& ret_val) const
{
  ret_val = value;
  return true;
}

bool IntValue::as_double_if_exists(const std::string& /*section_name*/, const std::string& /*entry_name*/,
                                   double& ret_val) const
{
  ret_val = static_cast<double>(value);
  return true;
}

void IntValue::print(std::ostream& output_stream) const { output_stream << value; }

class DoubleValue : public MTLValueBase
{
  private:
  const double value;

  public:
  DoubleValue(const double value_init);
  virtual ~DoubleValue();

  virtual bool as_double_if_exists(const std::string& section_name, const std::string& entry_name,
                                   double& ret_val) const;

  virtual void print(std::ostream& output_stream) const;
};

DoubleValue::DoubleValue(const double value_init) : MTLValueBase(), value(value_init)
{
  // Does nothing...
}

DoubleValue::~DoubleValue()
{
  // Does nothing...
}

bool DoubleValue::as_double_if_exists(const std::string& /*section_name*/, const std::string& /*entry_name*/,
                                      double& ret_val) const
{
  ret_val = value;
  return true;
}

void DoubleValue::print(std::ostream& output_stream) const { output_stream << value; }

class StringValue : public MTLValueBase
{
  private:
  const std::string value;

  public:
  StringValue(const std::string& value_init);
  virtual ~StringValue();

  virtual bool as_string_if_exists(const std::string& section_name, const std::string& entry_name,
                                   std::string& ret_val) const;

  virtual void print(std::ostream& output_stream) const;
};

StringValue::StringValue(const std::string& value_init) : MTLValueBase(), value(value_init)
{
  // Does nothing...
}

StringValue::~StringValue()
{
  // Does nothing...
}

bool StringValue::as_string_if_exists(const std::string& /*section_name*/, const std::string& /*entry_name*/,
                                      std::string& ret_val) const
{
  ret_val = value;
  return true;
}

void StringValue::print(std::ostream& output_stream) const
{
  const char quote = value.find('\'') == std::string::npos ? '\'' : '\"';
  output_stream << quote << value << quote;
}

class TupleValue : public MTLValueBase
{
  private:
  typedef std::vector<MTLValue*> ElementList;
  typedef ElementList::iterator ElementIter;
  typedef ElementList::const_iterator ConstElementIter;

  const ElementList elements;

  static void delete_element(MTLValue* const element);

  public:
  TupleValue(const std::vector<MTLValue*>& elements_init);
  virtual ~TupleValue();

  virtual bool as_int_tuple_if_exists(const std::string& section_name, const std::string& entry_name,
                                      IntTuple& ret_val) const;
  virtual bool as_double_tuple_if_exists(const std::string& section_name, const std::string& entry_name,
                                         DoubleTuple& ret_val) const;

  virtual void print(std::ostream& output_stream) const;
};

TupleValue::TupleValue(const std::vector<MTLValue*>& elements_init) : MTLValueBase(), elements(elements_init)
{
  // Does nothing...
}

TupleValue::~TupleValue() { std::for_each(elements.begin(), elements.end(), delete_element); }

void TupleValue::delete_element(MTLValue* const element) { delete element; }

bool TupleValue::as_int_tuple_if_exists(const std::string& section_name, const std::string& entry_name,
                                        IntTuple& ret_val) const
{
  ret_val.clear();
  ret_val.reserve(elements.size());

  const ConstElementIter first_element = elements.begin();
  const ConstElementIter last_element = elements.end();

  for (ConstElementIter i = first_element; i != last_element; ++i) {
    int element_value;
    const bool exists = (*i)->as_int_if_exists(section_name, entry_name, element_value);
    assert(exists);
    ret_val.push_back(element_value);
  }

  return true;
}

bool TupleValue::as_double_tuple_if_exists(const std::string& section_name, const std::string& entry_name,
                                           DoubleTuple& ret_val) const
{
  ret_val.clear();
  ret_val.reserve(elements.size());

  const ConstElementIter first_element = elements.begin();
  const ConstElementIter last_element = elements.end();

  for (ConstElementIter i = first_element; i != last_element; ++i) {
    double element_value;
    const bool exists = (*i)->as_double_if_exists(section_name, entry_name, element_value);
    assert(exists);
    ret_val.push_back(element_value);
  }

  return true;
}

void TupleValue::print(std::ostream& output_stream) const
{
  output_stream << "(";

  const ConstElementIter first_element = elements.begin();
  const ConstElementIter last_element = elements.end();

  for (ConstElementIter i = first_element; i != last_element; ++i) {
    if (i != first_element) {
      output_stream << ", ";
    }

    (*i)->print(output_stream);
  }

  output_stream << ")";
}

class EmptyValue : public MTLValue
{
  public:
  EmptyValue();
  virtual ~EmptyValue();

  virtual bool exists() const;

  virtual bool as_int_if_exists(const std::string& section_name, const std::string& entry_name, int& ret_val) const;
  virtual bool as_double_if_exists(const std::string& section_name, const std::string& entry_name,
                                   double& ret_val) const;
  virtual bool as_string_if_exists(const std::string& section_name, const std::string& entry_name,
                                   std::string& ret_val) const;
  virtual bool as_bool_if_exists(const std::string& section_name, const std::string& entry_name, bool& ret_val) const;
  virtual bool as_int_tuple_if_exists(const std::string& section_name, const std::string& entry_name,
                                      IntTuple& ret_val) const;
  virtual bool as_double_tuple_if_exists(const std::string& section_name, const std::string& entry_name,
                                         DoubleTuple& ret_val) const;

  virtual void print(std::ostream& output_stream) const;
};

EmptyValue::EmptyValue()
{
  // Does nothing...
}

EmptyValue::~EmptyValue()
{
  // Does nothing...
}

bool EmptyValue::exists() const { return false; }

bool EmptyValue::as_int_if_exists(const std::string& /*section_name*/, const std::string& /*entry_name*/,
                                  int& /*ret_val*/) const
{
  return false;
}

bool EmptyValue::as_double_if_exists(const std::string& /*section_name*/, const std::string& /*entry_name*/,
                                     double& /*ret_val*/) const
{
  return false;
}

bool EmptyValue::as_string_if_exists(const std::string& /*section_name*/, const std::string& /*entry_name*/,
                                     std::string& /*ret_val*/) const
{
  return false;
}

bool EmptyValue::as_bool_if_exists(const std::string& /*section_name*/, const std::string& /*entry_name*/,
                                   bool& /*ret_val*/) const
{
  return false;
}

bool EmptyValue::as_int_tuple_if_exists(const std::string& /*section_name*/, const std::string& /*entry_name*/,
                                        IntTuple& /*ret_val*/) const
{
  return false;
}

bool EmptyValue::as_double_tuple_if_exists(const std::string& /*section_name*/, const std::string& /*entry_name*/,
                                           DoubleTuple& /*ret_val*/) const
{
  return false;
}

void EmptyValue::print(std::ostream& /*output_stream*/) const { assert(false); }

// The value that is returned when a value that does not exist is requested.
const EmptyValue nonexistent_value;

void delete_value(MTLValue* const value) { delete value; }

void delete_entry(const MTLValueMap::value_type& entry) { delete entry.second; }

bool is_eof_or_newline(std::istream::int_type chr)
{
  return chr == std::istream::traits_type::eof() || chr == '\n' || chr == '\r';
}

bool is_eol(std::istream::int_type chr) { return is_eof_or_newline(chr) || chr == '#'; }

bool is_quote(std::istream::int_type chr) { return chr == '\'' || chr == '\"'; }

void skip_wspace(std::istream& input_stream)
{
  for (;;) {
    while (std::isspace(input_stream.peek())) {
      input_stream.get();
    }

    if (input_stream.peek() != '#') {
      break;
    }

    while (!is_eof_or_newline(input_stream.peek())) {
      input_stream.get();
    }
  }
}

bool is_ci_equal(const std::string& lhs, const std::string& rhs)
{
  if (lhs.length() != rhs.length()) {
    return false;
  }

  std::string::const_iterator i = lhs.begin();
  std::string::const_iterator j = rhs.begin();
  const std::string::const_iterator last = lhs.end();

  while (i != last) {
    if (std::tolower(*i++) != std::tolower(*j++)) {
      return false;
    }
  }

  return true;
}

std::vector<std::string> split_wspace(const std::string& textline)
{
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream token_str(textline);
  std::copy(std::istream_iterator<std::string>(token_str), std::istream_iterator<std::string>(),
            std::back_inserter(tokens));
  return tokens;
}

std::vector<std::string> split(const std::string& textline, char delimiter)
{
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream token_str(textline);
  while (std::getline(token_str, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}

MTLValue* read_number(const std::string& token)
{
  std::stringstream input_stream;
  input_stream << token;
  std::istream::int_type chr = input_stream.get();
  int sign = +1;

  switch (chr) {
  case '-':
    sign = -1;
  case '+':
    chr = input_stream.get();
  }

  int int_val = 0;
  double double_val = 0;

  // Read the value of the number as both an int and a double.
  while (std::isdigit(chr)) {
    int_val = int_val * 10 + chr - '0';
    double_val = double_val * 10 + chr - '0';
    chr = input_stream.get();
  }

  // If there is no radix point the number is considered to be an int.
  if (chr != '.') {
    input_stream.putback(chr);
    return new IntValue(sign * int_val);
  }

  chr = input_stream.get();
  double denom = 1;

  // Read the fractional part.
  while (std::isdigit(chr)) {
    double_val = double_val * 10 + chr - '0';
    denom *= 10;
    chr = input_stream.get();
  }

  input_stream.putback(chr);
  return new DoubleValue(sign * double_val / denom);
}

MTLValue* read_string(std::istream& input_stream)
{
  const std::istream::int_type quote = input_stream.get();
  assert(is_quote(quote));
  std::istream::pos_type pos = input_stream.tellg();
  std::istream::int_type chr = input_stream.get();
  std::string value = "";

  while (chr != quote) {
    // EOFs and newlines cannot occur in a string.
    if (is_eof_or_newline(chr)) {
      throw UnexpectedCharacter(chr, pos);
    }

    value += static_cast<char>(chr);
    pos = input_stream.tellg();
    chr = input_stream.get();
  }

  return new StringValue(value);
}

MTLValue* read_tuple(const std::vector<std::string>& tokens)
{

  std::vector<MTLValue*> elements;
  if (tokens.size() <= 1) {
    return new TupleValue(std::vector<MTLValue*>());
  }
  try {
    for (size_t t = 1; t < tokens.size(); t++) {
      std::string token = tokens.at(t);
      elements.push_back(read_number(token));
    }

  } catch (...) {
    // Free the already parsed elements.
    std::for_each(elements.begin(), elements.end(), delete_value);
    throw;
  }

  return new TupleValue(elements);
}

MTLValue* read_raw(std::istream& input_stream)
{
  std::istream::int_type chr = input_stream.get();
  std::string value = "";
  std::string::size_type last = 0;

  while (!is_eol(chr)) {
    value += static_cast<char>(chr);

    if (!std::isspace(chr)) {
      last = value.length();
    }

    chr = input_stream.get();
  }

  input_stream.putback(chr);
  value.erase(last);

  return new StringValue(value);
}

MTLValue* read_value(const std::vector<std::string>& tokens)
{

  std::stringstream input_stream;
  const size_t num_tokens = tokens.size();
  for (size_t t = 1; t < num_tokens; t++) {
    input_stream << tokens.at(t);
  }
  const char chr = input_stream.peek();
  if (std::isdigit(chr) || chr == '+' || chr == '-') {
    if (num_tokens == 2) {
      return read_number(tokens.at(1));
    } else {
      return read_tuple(tokens);
    }
  } else if (is_quote(chr)) {
    return read_string(input_stream);
  } else if (is_eol(chr)) {
    return new StringValue("");
  }

  return read_raw(input_stream);
}

bool peek_ci_equals(const std::string& str, std::istream& input_stream)
{
  std::string removed;
  bool equal = true;
  for (size_t i = 0; i < str.size(); i++) {
    char cs = str.at(i);
    char ci = input_stream.peek();
    if (ci != cs) {
      equal = false;
      break;
    }
    input_stream.get();
    removed.insert(removed.begin(), ci);
  }
  // Put the chars back
  for (const char& c : removed) {
    input_stream.putback(c);
  }
  return equal;
}

MTLValueMap read_entries(const std::string& name, std::istream& input_stream)
{
  MTLValueMap values;

  std::string line;
  try {
    while (input_stream.peek() != std::istream::traits_type::eof() && !peek_ci_equals("newmtl", input_stream)) {
      std::getline(input_stream, line);
      auto comment_start = line.find_first_of('#');
      if (comment_start != std::string::npos) {
        try {
          line = line.substr(0, comment_start);
        } catch (...) {
          throw MalformedComment(line);
        }
      }
      if (!line.empty()) {
        const std::vector<std::string>& tokens = split_wspace(line);

        if (tokens.size() == 0) {
          throw NonexistentEntry(name, "<empty line>");
        }
        std::string key = tokens.at(0);
        if (tokens.size() == 1) {
          throw NonexistentEntry(name, key);
        }
        std::auto_ptr<MTLValue> value(read_value(tokens));

        if (!values.insert(std::make_pair(key, value.get())).second) {
          throw DuplicateEntry(name, key);
        }

        value.release();
        skip_wspace(input_stream);
      }
    }
  } catch (...) {
    std::for_each(values.begin(), values.end(), delete_entry);
    throw; // Re-throw the exception.
  }

  return values;
}
} // namespace

MTLEntry::MTLEntry(const std::string& material_name_init, const std::string& entry_name_init,
                   const MTLValue* const value_ptr_init)
    : material_name(material_name_init), entry_name(entry_name_init), value_ptr(value_ptr_init)
{
  // Does nothing...
}

MTLEntry::MTLEntry(const MTLEntry& original)
    : material_name(original.material_name), entry_name(original.entry_name), value_ptr(original.value_ptr)
{
  // Does nothing...
}

MTLEntry::~MTLEntry()
{
  // Does nothing...
}

MTLEntry& MTLEntry::operator=(const MTLEntry& original)
{
  material_name = original.material_name;
  entry_name = original.entry_name;
  value_ptr = original.value_ptr;

  return *this;
}

const std::string& MTLEntry::get_mtl_name() const { return material_name; }

const std::string& MTLEntry::get_entry_name() const { return entry_name; }

bool MTLEntry::exists() const { return value_ptr->exists(); }

bool MTLEntry::as_int_if_exists(int& ret_val) const
{
  return value_ptr->as_int_if_exists(material_name, entry_name, ret_val);
}

bool MTLEntry::as_double_if_exists(double& ret_val) const
{
  return value_ptr->as_double_if_exists(material_name, entry_name, ret_val);
}

bool MTLEntry::as_string_if_exists(std::string& ret_val) const
{
  return value_ptr->as_string_if_exists(material_name, entry_name, ret_val);
}

bool MTLEntry::as_int_tuple_if_exists(IntTuple& ret_val) const
{
  return value_ptr->as_int_tuple_if_exists(material_name, entry_name, ret_val);
}

bool MTLEntry::as_double_tuple_if_exists(DoubleTuple& ret_val) const
{
  return value_ptr->as_double_tuple_if_exists(material_name, entry_name, ret_val);
}

int MTLEntry::as_int_or_die() const
{
  int value;

  if (as_int_if_exists(value)) {
    return value;
  }

  throw NonexistentEntry(material_name, entry_name);
}

double MTLEntry::as_double_or_die() const
{
  double value;

  if (as_double_if_exists(value)) {
    return value;
  }

  throw NonexistentEntry(material_name, entry_name);
}

std::string MTLEntry::as_string_or_die() const
{
  std::string value;

  if (as_string_if_exists(value)) {
    return value;
  }

  throw NonexistentEntry(material_name, entry_name);
}

IntTuple MTLEntry::as_int_tuple_or_die() const
{
  IntTuple value;

  if (as_int_tuple_if_exists(value)) {
    return value;
  }

  throw NonexistentEntry(material_name, entry_name);
}

DoubleTuple MTLEntry::as_double_tuple_or_die() const
{
  DoubleTuple value;

  if (as_double_tuple_if_exists(value)) {
    return value;
  }

  throw NonexistentEntry(material_name, entry_name);
}

int MTLEntry::as_int_or_default(const int def_val) const
{
  int value;

  if (as_int_if_exists(value)) {
    return value;
  }

  return def_val;
}

double MTLEntry::as_double_or_default(const double def_val) const
{
  double value;

  if (as_double_if_exists(value)) {
    return value;
  }

  return def_val;
}

std::string MTLEntry::as_string_or_default(const std::string& def_val) const
{
  std::string value;

  if (as_string_if_exists(value)) {
    return value;
  }

  return def_val;
}

IntTuple MTLEntry::as_int_tuple_or_default(const IntTuple& def_val) const
{
  IntTuple value;

  if (as_int_tuple_if_exists(value)) {
    return value;
  }

  return def_val;
}

DoubleTuple MTLEntry::as_double_tuple_or_default(const DoubleTuple& def_val) const
{
  DoubleTuple value;

  if (as_double_tuple_if_exists(value)) {
    return value;
  }

  return def_val;
}

MTLEntry::operator int() const { return as_int_or_die(); }

MTLEntry::operator double() const { return as_double_or_die(); }

MTLEntry::operator std::string() const { return as_string_or_die(); }

MTLEntry::operator IntTuple() const { return as_int_tuple_or_die(); }

MTLEntry::operator DoubleTuple() const { return as_double_tuple_or_die(); }

int MTLEntry::operator||(const int def_val) const { return as_int_or_default(def_val); }

double MTLEntry::operator||(const double def_val) const { return as_double_or_default(def_val); }

std::string MTLEntry::operator||(const std::string& def_val) const { return as_string_or_default(def_val); }

IntTuple MTLEntry::operator||(const IntTuple& def_val) const { return as_int_tuple_or_default(def_val); }

DoubleTuple MTLEntry::operator||(const DoubleTuple& def_val) const { return as_double_tuple_or_default(def_val); }

Material::Material(const std::string& section_name_init, const MTLValueMap* const values_init)
    : material_name(section_name_init), values(values_init)
{
  // Does nothing...
}

Material::Material(const Material& original) : material_name(original.material_name), values(original.values)
{
  // Does nothing...
}

Material::~Material()
{
  // Does nothing...
}

MTLEntry Material::operator[](const std::string& key) const
{
  const ConstMTLValueIter value = values->find(key);

  if (value == values->end()) {
    return MTLEntry(material_name, key, &nonexistent_value);
  }

  return MTLEntry(material_name, key, &*value->second);
}

namespace {
/*
 * The material that is returned when the requested material does not exists.  This
 * material does not contain any values so all the values that are retrieved from
 * this material will be EmptyValues.
 */
const MTLValueMap empty_section;
} // namespace

MTLLibrary::MTLLibrary() : materials()
{
  // Does nothing...
}

MTLLibrary::MTLLibrary(std::istream& input_stream) : materials() { parse(input_stream); }

MTLLibrary::MTLLibrary(const MTLLibrary& /*original*/)
{
  // This copy constructor should not be used.
  std::terminate();
}

MTLLibrary::~MTLLibrary()
{
  // Delete all materials in this configuration.
  std::for_each(materials.begin(), materials.end(), delete_material);
}

MTLLibrary& MTLLibrary::operator=(const MTLLibrary& /*original*/)
{
  // This copy-assignment operator should not be used.
  std::terminate();
  return *this;
}

void MTLLibrary::delete_material(const MaterialMap::value_type& section)
{
  std::for_each(section.second.begin(), section.second.end(), delete_entry);
}

Material MTLLibrary::operator[](const std::string& name) const
{
  const ConstMaterialIter section = materials.find(name);

  // Return a section containing no values if the
  // section does not exist.
  if (section == materials.end()) {
    return Material(name, &empty_section);
  }

  return Material(name, &section->second);
}

void MTLLibrary::parse(std::istream& input_stream)
{

  try {
    std::string line;
    while (std::getline(input_stream, line) && input_stream.good()) {
      auto comment_start = line.find_first_of('#');
      if (comment_start != std::string::npos) {
        try {
          line = line.substr(0, comment_start);
        } catch (...) {
          throw MalformedComment(line);
        }
      }
      if (!line.empty()) {
        std::vector<std::string> tokens = split_wspace(line);
        std::string key = tokens.at(0);
        if (is_ci_equal(key, "newmtl")) {
          const std::string name = tokens.at(1);
          const MTLValueMap mats = read_entries(name, input_stream);

          if (!materials.insert(std::make_pair(name, mats)).second) {
            throw DuplicateMaterial(name);
          }
        }
      }
    }
  } catch (...) {
    // Throw to main
    throw;
  }
}
void MTLLibrary::print(std::ostream& output_stream) const
{
  const ConstMaterialIter first_section = materials.begin();
  const ConstMaterialIter last_section = materials.end();

  for (ConstMaterialIter i = first_section; i != last_section; ++i) {
    // Print a blank line between sections.
    if (i != first_section) {
      output_stream << std::endl;
    }

    /* Print the header of the section. */
    output_stream << "[" << i->first << "]" << std::endl;

    const ConstMTLValueIter first_entry = i->second.begin();
    const ConstMTLValueIter last_entry = i->second.end();

    // Print the entries in the section.
    for (ConstMTLValueIter j = first_entry; j != last_entry; ++j) {
      output_stream << j->first << " = ";
      j->second->print(output_stream);
      output_stream << std::endl;
    }
  }
}

std::istream& operator>>(std::istream& input_stream, MTLLibrary& configuration)
{
  configuration.parse(input_stream);
  return input_stream;
}

std::ostream& operator<<(std::ostream& output_stream, const MTLLibrary& mtllib)
{
  mtllib.print(output_stream);
  return output_stream;
}

Polygon::Polygon(const IntTuple& indexes_init, const IntTuple& texture_indexes_init,
                 const IntTuple& normal_indexes_init)
    : indexes(indexes_init), texture_indexes(texture_indexes_init), normal_indexes(normal_indexes_init)
{
}

const IntTuple& Polygon::get_indexes() const { return indexes; }
IntTuple Polygon::get_indexes() { return indexes; }

const IntTuple& Polygon::get_texture_indexes() const { return texture_indexes; }
IntTuple Polygon::get_texture_indexes() { return texture_indexes; }

const IntTuple& Polygon::get_normal_indexes() const { return normal_indexes; }
IntTuple Polygon::get_normal_indexes() { return normal_indexes; }

bool Polygon::has_texture_indexes() const { return !texture_indexes.empty(); }
bool Polygon::has_normal_indexes() const { return !normal_indexes.empty(); }

void ObjectGroup::parse_vertex(const std::vector<std::string>& tokens, const std::string& line)
{
  const size_t num_tokens = tokens.size();
  if (num_tokens == 4) {
    const double x = std::stod(tokens.at(1));
    const double y = std::stod(tokens.at(2));
    const double z = std::stod(tokens.at(3));
    vertexes.push_back({x, y, z});
  } else if (num_tokens == 5) {
    const double x = std::stod(tokens.at(1));
    const double y = std::stod(tokens.at(2));
    const double z = std::stod(tokens.at(3));
    const double w = std::stod(tokens.at(4));
    vertexes.push_back({x, y, z, w});
  } else {
    throw MalformedVertex(line);
  }
}
void ObjectGroup::parse_vertex_normal(const std::vector<std::string>& tokens, const std::string& line)

{
  if (tokens.size() == 4) {
    const double x = std::stod(tokens.at(1));
    const double y = std::stod(tokens.at(2));
    const double z = std::stod(tokens.at(3));
    vertex_normals.push_back({x, y, z});
  } else {
    throw MalformedVertex(line);
  }
}
void ObjectGroup::parse_texture_coordinates(const std::vector<std::string>& tokens, const std::string& line)
{
  double u, v, w;
  DoubleTuple tup;
  switch (tokens.size()) {
  case 4:
    w = std::stod(tokens.at(3));
    tup.insert(tup.begin(), w);
  case 3:
    v = std::stod(tokens.at(2));
    tup.insert(tup.begin(), v);
  case 2:
    u = std::stod(tokens.at(1));
    tup.insert(tup.begin(), u);
    break;
  default:
    throw MalformedVertex(line);
  }
  texture_coordinates.push_back(tup);
}

void ObjectGroup::parse_polygon(const std::vector<std::string>& tokens, const std::string& line)

{
  IntTuple indexes;
  IntTuple norm_coord;
  IntTuple tex_coord;
  for (unsigned int i = 1; i < tokens.size(); i++) {
    auto index_it = split(tokens.at(i), '/');
    const size_t num_indexes = index_it.size();
    if (num_indexes >= 1) {
      indexes.emplace_back(std::stoi(index_it.at(0)));
    }
    if (num_indexes >= 2 && !index_it.at(1).empty()) {
      tex_coord.emplace_back(std::stoi(index_it.at(1)));
    }
    if (num_indexes == 3 && !index_it.at(2).empty()) {
      norm_coord.emplace_back(std::stoi(index_it.at(2)));
    }
    if (num_indexes > 4 || num_indexes <= 0) {
      throw MalformedPolygon(line);
    }
  }
  polygons.emplace_back(Polygon{indexes, tex_coord, norm_coord});
}

void ObjectGroup::parse_mtllib(const std::vector<std::string>& tokens, const std::string& line)
{

  if (tokens.size() != 2) {
    throw MalformedLine("mtllib", line);
  }
  if (!mtllib_filename.empty()) {
    throw DuplicateEntry("object group", "mtllib");
  } else {
    mtllib_filename = tokens.at(1);
  }
}

void ObjectGroup::parse_usemtl(const std::vector<std::string>& tokens, const std::string& line)
{

  if (tokens.size() != 2) {
    throw MalformedLine("usemtl", line);
  }
  if (!mtl_name.empty()) {
    throw DuplicateLine("usemtl");
  } else {
    mtl_name = tokens.at(1);
  }
}

void ObjectGroup::parse_obj_line(const std::string& line)
{
  auto tokens = split_wspace(line);
  const std::string prefix = tokens.at(0);
  if (is_ci_equal(prefix, "vn")) {
    parse_vertex_normal(tokens, line);
  } else if (is_ci_equal(prefix, "vt")) {
    parse_texture_coordinates(tokens, line);
  } else if (is_ci_equal(prefix, "v")) {
    parse_vertex(tokens, line);
  } else if (is_ci_equal(prefix, "l")) {
    // polygons.push_back({std::stoul(tokens.at(1)), std::stoul(tokens.at(2))});
  } else if (is_ci_equal(prefix, "f")) {
    parse_polygon(tokens, line);
  } else if (is_ci_equal(prefix, "mtllib")) {
    parse_mtllib(tokens, line);
  } else if (is_ci_equal(prefix, "usemtl")) {
    parse_usemtl(tokens, line);
  } else {
    throw UnrecognizedType(line);
  }
}

ObjectGroup::ObjectGroup(std::istream& input_stream) { parse(input_stream); }

void ObjectGroup::print(std::ostream& output_stream) const
{
  output_stream << "[Object group]" << std::endl;
  output_stream << " # Vertexes: " << vertexes.size() << std::endl;
  output_stream << " # Texture coordinates: " << texture_coordinates.size() << std::endl;
  output_stream << " # Vertex normals: " << vertex_normals.size() << std::endl;
  output_stream << " # Polygons: " << polygons.size() << std::endl;
  if (!mtl_name.empty())
    output_stream << " Using material: '" << mtl_name << "'" << std::endl;
  if (!mtllib_filename.empty())
    output_stream << " Using material library: '" << mtllib_filename << "'" << std::endl;
}

std::ostream& operator<<(std::ostream& output_stream, const ObjectGroup& group)
{
  group.print(output_stream);
  return output_stream;
}

void ObjectGroup::parse(std::istream& input_stream)
{
  try {
    std::string line;
    while (std::getline(input_stream, line) && input_stream.good()) {
      auto comment_start = line.find_first_of('#');
      if (comment_start != std::string::npos) {
        try {
          line = line.substr(0, comment_start);
        } catch (...) {
          throw MalformedComment(line);
        }
      }
      if (!line.empty()) {
        parse_obj_line(line);
      }
    }
  } catch (...) {
    // Throw to main
    throw;
  }
}

std::vector<DoubleTuple> ObjectGroup::get_vertexes() { return vertexes; }
std::vector<DoubleTuple> ObjectGroup::get_vertex_normals() { return vertex_normals; }
std::vector<DoubleTuple> ObjectGroup::get_texture_coordinates() { return texture_coordinates; }
std::vector<Polygon> ObjectGroup::get_polygons() { return polygons; }
std::string ObjectGroup::get_mtl_name() { return mtl_name; }
std::string ObjectGroup::get_mtllib_file_name() { return mtllib_filename; }

const std::vector<DoubleTuple>& ObjectGroup::get_vertexes() const { return vertexes; }
const std::vector<DoubleTuple>& ObjectGroup::get_vertex_normals() const { return vertex_normals; }
const std::vector<DoubleTuple>& ObjectGroup::get_texture_coordinates() const { return texture_coordinates; }
const std::vector<Polygon>& ObjectGroup::get_polygons() const { return polygons; }
const std::string& ObjectGroup::get_mtl_name() const { return mtl_name; }
const std::string& ObjectGroup::get_mtllib_file_name() const { return mtllib_filename; }

ObjectGroup OBJFile::get_object() { return object; }

const ObjectGroup& OBJFile::get_object() const { return object; }

void OBJFile::parse(std::istream& input_stream) { object.parse(input_stream); }

void OBJFile::print(std::ostream& output_stream) const { output_stream << object << std::endl; }

std::istream& operator>>(std::istream& input_stream, OBJFile& file)
{
  file.parse(input_stream);
  return input_stream;
}

std::ostream& operator<<(std::ostream& output_stream, const OBJFile& file)
{
  file.print(output_stream);
  return output_stream;
}

} // namespace obj
