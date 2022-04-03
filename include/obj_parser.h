/*
 * obj_parser.h
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

#ifndef OBJ_PARSER_INCLUDED
#define OBJ_PARSER_INCLUDED

#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * \brief The namespace of the OBJ file parser.
 */
namespace obj {
/**
 * \brief The base class for all exceptions that are thrown by the OBJ-parser.
 */
class ParseException : public std::exception
{
  protected:
  /**
   * \brief Constructs a new ParseException instance.
   */
  ParseException() throw();

  /**
   * \brief Constructs a new ParseException instance by copying another one.
   *
   * \param original The exception that is copied.
   */
  ParseException(const ParseException& original) throw();

  /**
   * \brief Destructs a ParseException.
   */
  virtual ~ParseException() throw();

  /**
   * \brief Copies a parse exception.
   *
   * \param original The parse exception that is copied.
   *
   * \return A reference to this instance.
   */
  ParseException& operator=(const ParseException& original) throw();

  public:
  /**
   * \brief Returns a description of the error that occurred.
   *
   * \return A description of the error that occurred.
   */
  virtual const char* what() const throw() = 0;
};

/**
 * \brief The exception that is thrown when the parser encounters an unexpected character.
 */
class UnexpectedCharacter : public ParseException
{
  private:
  /**
   * \brief The unexpected character.
   */
  std::istream::int_type character;

  /**
   * \brief The position of the character in the input stream.
   */
  std::istream::pos_type position;

  /**
   * \brief The message that is returned by the what() method.
   */
  std::string message;

  public:
  /**
   * \brief Constructs a new UnexpectedCharacter exception.
   *
   * \param character_init The unexpected character.
   * \param position_init  The position of the character in the input stream.
   */
  UnexpectedCharacter(const std::istream::int_type character_init, const std::istream::pos_type position_init) throw();

  /**
   * \brief Constructs a new UnexpectedCharacter instance by copying another one.
   *
   * \param original The instance that is copied.
   */
  UnexpectedCharacter(const UnexpectedCharacter& original) throw();

  /**
   * \brief Destructs a .
   */
  virtual ~UnexpectedCharacter() throw();

  /**
   * \brief Copies an UnexpectedCharacter.
   *
   * \param original The instance that is copied.
   *
   * \return A reference to this instance.
   */
  UnexpectedCharacter& operator=(const UnexpectedCharacter& original) throw();

  /**
   * \brief Returns a description of the error that occurred.
   *
   * \return A description of the error hat occurred.
   */
  virtual const char* what() const throw();
};

/**
 * \brief The exception that is thrown by the parser when it encounters a material that has the same name as a
 * previously parsed section.
 */
class DuplicateMaterial : public ParseException
{
  private:
  /**
   * \brief The name of the duplicate material.
   */
  std::string name;

  /**
   * \brief The message that is returned by the what() method.
   */
  std::string message;

  public:
  /**
   * \brief Constructs a new DuplicateMaterial instance.
   *
   * \param name_init The name of the duplicate section.
   */
  DuplicateMaterial(const std::string& name_init) throw();

  /**
   * \brief Constructs a new DuplicateMaterial instance by copying another one.
   *
   * \param original The instance that is copied.
   */
  DuplicateMaterial(const DuplicateMaterial& original) throw();

  /**
   * \brief Destructs a DuplicateMaterial.
   */
  virtual ~DuplicateMaterial() throw();

  /**
   * \brief Copies a DuplicateMaterial.
   *
   * \param original The instance that is copied.
   *
   * \return A reference to this instance.
   */
  DuplicateMaterial& operator=(const DuplicateMaterial& original) throw();

  /**
   * \brief Returns a description of the error that occurred.
   *
   * \return A description of the error hat occurred.
   */
  virtual const char* what() const throw();
};

/**
 * \brief The exception that is thrown by the parser when it encounters insufficiently specified coordinates.
 */
class MalformedVertex : public ParseException
{
  private:
  /**
   * \brief The line of the vertex or coordinates.
   */
  std::string line;

  /**
   * \brief The message that is returned by the what() method.
   */
  std::string message;

  public:
  /**
   * \brief Constructs a new MalformedVertex instance.
   *
   * \param line_init The line containing the coordinates.
   */
  MalformedVertex(const std::string& name_init) throw();

  /**
   * \brief Constructs a new MalformedVertex instance by copying another one.
   *
   * \param original The instance that is copied.
   */
  MalformedVertex(const MalformedVertex& original) throw();

  /**
   * \brief Destructs a MalformedVertex.
   */
  virtual ~MalformedVertex() throw();

  /**
   * \brief Copies a MalformedVertex.
   *
   * \param original The instance that is copied.
   *
   * \return A reference to this instance.
   */
  MalformedVertex& operator=(const MalformedVertex& original) throw();

  /**
   * \brief Returns a description of the error that occurred.
   *
   * \return A description of the error hat occurred.
   */
  virtual const char* what() const throw();
};

/**
 * \brief The exception that is thrown by the parser when it encounters an insufficiently specified polygon.
 */
class MalformedPolygon : public ParseException
{
  private:
  /**
   * \brief The line of the polygon.
   */
  std::string line;

  /**
   * \brief The message that is returned by the what() method.
   */
  std::string message;

  public:
  /**
   * \brief Constructs a new MalformedPolygon instance.
   *
   * \param line_init The line containing the polygon.
   */
  MalformedPolygon(const std::string& name_init) throw();

  /**
   * \brief Constructs a new MalformedPolygon instance by copying another one.
   *
   * \param original The instance that is copied.
   */
  MalformedPolygon(const MalformedPolygon& original) throw();

  /**
   * \brief Destructs a MalformedPolygon.
   */
  virtual ~MalformedPolygon() throw();

  /**
   * \brief Copies a MalformedPolygon.
   *
   * \param original The instance that is copied.
   *
   * \return A reference to this instance.
   */
  MalformedPolygon& operator=(const MalformedPolygon& original) throw();

  /**
   * \brief Returns a description of the error that occurred.
   *
   * \return A description of the error hat occurred.
   */
  virtual const char* what() const throw();
};

/**
 * \brief The exception that is thrown by the parser when it encounters a malformed comment on a line.
 */
class MalformedComment : public ParseException
{
  private:
  /**
   * \brief The line of the comment.
   */
  std::string line;

  /**
   * \brief The message that is returned by the what() method.
   */
  std::string message;

  public:
  /**
   * \brief Constructs a new MalformedComment instance.
   *
   * \param line_init The line containing the comment.
   */
  MalformedComment(const std::string& name_init) throw();

  /**
   * \brief Constructs a new MalformedComment instance by copying another one.
   *
   * \param original The instance that is copied.
   */
  MalformedComment(const MalformedComment& original) throw();

  /**
   * \brief Destructs a MalformedComment.
   */
  virtual ~MalformedComment() throw();

  /**
   * \brief Copies a MalformedComment.
   *
   * \param original The instance that is copied.
   *
   * \return A reference to this instance.
   */
  MalformedComment& operator=(const MalformedComment& original) throw();

  /**
   * \brief Returns a description of the error that occurred.
   *
   * \return A description of the error hat occurred.
   */
  virtual const char* what() const throw();
};

/**
 * \brief The exception that is thrown when the parser encounters an entry that has the same key as a previously parsed
 * key in the same section.
 */
class DuplicateEntry : public ParseException
{
  private:
  /**
   * \brief The name of the section that contains the duplicate entry.
   */
  std::string section;

  /**
   * \brief The name of the duplicate entry.
   */
  std::string key;

  /**
   * \brief The message that is returned by the what() method.
   */
  std::string message;

  public:
  /**
   * \brief Constructs a new DuplicateEntry instance.
   *
   * \param section_init The name of the section that contains the duplicate entry.
   * \param key_init     The name of the duplicate entry.
   */
  DuplicateEntry(const std::string& section_init, const std::string& key_init) throw();

  /**
   * \brief Constructs a new DuplicateEntry instance by copying another one.
   *
   * \param original The instance that is copied.
   */
  DuplicateEntry(const DuplicateEntry& original) throw();

  /**
   * \brief Destructs a DuplicateEntry.
   */
  virtual ~DuplicateEntry() throw();

  /**
   * \brief Copies a DuplicateEntry exception.
   *
   * \param original The instance that is copied.
   *
   * \return A reference to this instance.
   */
  DuplicateEntry& operator=(const DuplicateEntry& original) throw();

  /**
   * \brief Returns a description of the error hat occurred.
   *
   * \return A description of the error hat occurred.
   */
  virtual const char* what() const throw();
};

/**
 * \brief The execption that is thrown when the value of a nonexistent entry is requested.
 *
 * Note that this exception is only thrown when the value of an Entry is obtained; not when the entry is obtained from a
 * Section.
 */
class NonexistentEntry : public std::exception
{
  private:
  /**
   * \brief The name of the section from which the nonexistent entry is obtained.
   */
  std::string section_name;

  /**
   * \brief The name of the nonexistent entry.
   */
  std::string entry_name;

  /**
   * \brief The message that is returned by the what() method.
   */
  std::string message;

  public:
  /**
   * \brief Constructs a new NonexistentEntry instance.
   *
   * \param section_name_init The name of the section from which the nonexistent entry is obtained.
   * \param entry_name_init   The name of the nonexistent entry.
   */
  NonexistentEntry(const std::string& section_name_init, const std::string& entry_name_init) throw();

  /**
   * \brief Constructs a new NonexistentEntry instance by copying another one.
   *
   * \param original The instance that is copied.
   */
  NonexistentEntry(const NonexistentEntry& original) throw();

  /**
   * \brief Destructs a NonexistentEntry.
   */
  virtual ~NonexistentEntry() throw();

  /**
   * \brief Copies a NonexistentEntry.
   *
   * \param original The instance that is copied.
   *
   * \return A reference to this instance.
   */
  NonexistentEntry& operator=(const NonexistentEntry& original) throw();

  /**
   * \brief Returns a description of the error hat occurred.
   *
   * \return A description of the error hat occurred.
   */
  const char* what() const throw();
};

/**
 * \brief The execption that is thrown when the value of an entry in an ini-configuration cannot be converted to the
 * requested value.
 */
class IncompatibleConversion : public std::exception
{
  private:
  /**
   * \brief The name of the section of the entry.
   */
  std::string section_name;

  /**
   * \brief The name of the entry.
   */
  std::string entry_name;

  /**
   * \brief The name of the requested type, e.g. "int".
   */
  std::string type_name;

  /**
   * \brief The message that is returned by the what() method.
   */
  std::string message;

  public:
  /**
   * \brief Constructs a new IncompatibleConversion exception.
   *
   * \param section_name_init The name of the section of the entry.
   * \param entry_name_init   The name of the entry.
   * \param type_name_init    The name of the requested type, e.g. "int".
   */
  IncompatibleConversion(const std::string& section_name_init, const std::string& entry_name_init,
                         const std::string& type_name_init) throw();

  /**
   * \brief Constructs a IncompatibleConversion exception by copying another one.
   *
   * \param original The instance that is copied.
   */
  IncompatibleConversion(const IncompatibleConversion& original) throw();

  /**
   * \brief Destructs a .
   */
  virtual ~IncompatibleConversion() throw();

  /**
   * \brief Copies an IncompatibleConversion.
   *
   * \param original The instance that is copied.
   *
   * \return A reference to this instance.
   */
  IncompatibleConversion& operator=(const IncompatibleConversion& original) throw();

  /**
   * \brief Returns a description of the error hat occurred.
   *
   * \return A description of the error hat occurred.
   */
  virtual const char* what() const throw();
};

/**
 * \brief The exception that is thrown by the parser when it encounters an unrecognized line type.
 */
class UnrecognizedType : public ParseException
{
  private:
  /**
   * \brief The unrecognized.
   */
  std::string line;

  /**
   * \brief The message that is returned by the what() method.
   */
  std::string message;

  public:
  /**
   * \brief Constructs a new UnrecognizedType instance.
   *
   * \param line_init The unrecognized line.
   */
  UnrecognizedType(const std::string& name_init) throw();

  /**
   * \brief Constructs a new UnrecognizedType instance by copying another one.
   *
   * \param original The instance that is copied.
   */
  UnrecognizedType(const UnrecognizedType& original) throw();

  /**
   * \brief Destructs a UnrecognizedType.
   */
  virtual ~UnrecognizedType() throw();

  /**
   * \brief Copies a UnrecognizedType.
   *
   * \param original The instance that is copied.
   *
   * \return A reference to this instance.
   */
  UnrecognizedType& operator=(const UnrecognizedType& original) throw();

  /**
   * \brief Returns a description of the error that occurred.
   *
   * \return A description of the error hat occurred.
   */
  virtual const char* what() const throw();
};

/**
 * \brief The exception that is thrown by the parser when it encounters a duplicate line for a key.
 */
class DuplicateLine : public ParseException
{
  private:
  /**
   * \brief The unrecognized.
   */
  std::string line;

  /**
   * \brief The message that is returned by the what() method.
   */
  std::string message;

  public:
  /**
   * \brief Constructs a new DuplicateLine instance.
   *
   * \param line_init The duplicate line.
   */
  DuplicateLine(const std::string& name_init) throw();

  /**
   * \brief Constructs a new DuplicateLine instance by copying another one.
   *
   * \param original The instance that is copied.
   */
  DuplicateLine(const DuplicateLine& original) throw();

  /**
   * \brief Destructs a DuplicateLine.
   */
  virtual ~DuplicateLine() throw();

  /**
   * \brief Copies a DuplicateLine.
   *
   * \param original The instance that is copied.
   *
   * \return A reference to this instance.
   */
  DuplicateLine& operator=(const DuplicateLine& original) throw();

  /**
   * \brief Returns a description of the error that occurred.
   *
   * \return A description of the error hat occurred.
   */
  virtual const char* what() const throw();
};

/**
 * \brief The exception that is thrown by the parser when it encounters a malformed line.
 */
class MalformedLine : public ParseException
{
  private:
  /**
   * \brief The malformed line.
   */
  std::string line;

  /**
   * \brief The message that is returned by the what() method.
   */
  std::string message;

  public:
  /**
   * \brief Constructs a new MalformedLine instance.
   *
   * \param line_init The unrecognized line.
   */
  MalformedLine(const std::string& name_init, const std::string& line_init) throw();

  /**
   * \brief Constructs a new MalformedLine instance by copying another one.
   *
   * \param original The instance that is copied.
   */
  MalformedLine(const MalformedLine& original) throw();

  /**
   * \brief Destructs a MalformedLine.
   */
  virtual ~MalformedLine() throw();

  /**
   * \brief Copies a MalformedLine.
   *
   * \param original The instance that is copied.
   *
   * \return A reference to this instance.
   */
  MalformedLine& operator=(const MalformedLine& original) throw();

  /**
   * \brief Returns a description of the error that occurred.
   *
   * \return A description of the error hat occurred.
   */
  virtual const char* what() const throw();
};

/**
 * \brief The type that is used to store int tuples (list of ints).
 */
using IntTuple = std::vector<int>;

/**
 * \brief The type that is used to store double tuples (list of doubles).
 */
using DoubleTuple = std::vector<double>;

/**
 * \brief A polygon that is part of a mesh. This class only keeps track of the indexes of the points/texture
 * coordinates/vector normals of the polygon, and not the coordinates themselves.
 */
class Polygon
{
  private:
  /**
   * \brief Vertex/point indices
   */
  IntTuple indexes;
  /**
   * \brief Texture coordinate indexes
   */
  IntTuple texture_indexes;
  /**
   * \brief Vertex normal indexes.
   */
  IntTuple normal_indexes;

  public:
  /**
   * \brief Constructs a polygon
   *
   * \param indexes Vertex indexes of points
   *
   * \param texture_indexes Indexes of texture coordinates
   *
   * \param normal_indexes Indexes of vertex normals
   */
  Polygon(const IntTuple& indexes, const IntTuple& texture_indexes, const IntTuple& normal_indexes);

  const IntTuple& get_indexes() const;
  IntTuple get_indexes();

  const IntTuple& get_texture_indexes() const;
  IntTuple get_texture_indexes();

  const IntTuple& get_normal_indexes() const;
  IntTuple get_normal_indexes();

  bool has_texture_indexes() const;
  bool has_normal_indexes() const;
};

/**
 * \brief The base class for all classes that contain the value of an entry.
 */
class MTLValue;

/**
 * \brief The class that represents an entry in the material of a Material Library.
 */
class MTLEntry
{
  private:
  /**
   * \brief The name of the material to which this entry belongs.
   */
  std::string material_name;

  /**
   * \brief The name of this entry.
   */
  std::string entry_name;

  /**
   * \brief A pointer to the value of this entry.
   */
  const MTLValue* value_ptr;

  public:
  /**
   * \brief Constructs a new entry given the name of the material it belongs to and its value.
   *
   * \param material_name_init The name of the material to which this entry belongs.
   * \param entry_name_init The name of this entry.
   * \param value_ptr_init A pointer to the value of this entry.
   */
  MTLEntry(const std::string& material_name_init, const std::string& entry_name_init,
           const MTLValue* const value_ptr_init);

  /**
   * \brief Constructs an entry by copying another one.
   *
   * \param original The section whose values are copied.
   */
  MTLEntry(const MTLEntry& original);

  /**
   * \brief Destructs an entry.
   */
  ~MTLEntry();

  /**
   * \brief Copies an entry.
   *
   * \param original The section whose values are copied.
   *
   * \return A reference to this entry.
   */
  MTLEntry& operator=(const MTLEntry& original);

  /**
   * \brief Returns the name of the material to which this entry belongs.
   *
   * \return The name of the material to which this entry belongs.
   */
  const std::string& get_mtl_name() const;

  /**
   * \brief Returns the name of this entry.
   *
   * \return Returns the name of this entry.
   */
  const std::string& get_entry_name() const;

  /**
   * \brief Checks whether this entry exists in the material or not.
   *
   * \return \c true if this entry exits, \c false otherwise.
   */
  bool exists() const;

  /**
   * \brief Returns the value as an int.
   *
   * If the entry exists and can be represented as an int, the value is passed to
   * the caller through the parameter and \c true is returned.  If the entry exists
   * but is not representable as an int, an IncompatibleConversion exception is
   * thrown.  If the entry does not exist, \c false is returned and the value of
   * the parameter is not changed.
   *
   * \param ret_val The parameter through which the value is returned.
   *
   * \return \c true if the value exists, \c false otherwise.
   */
  bool as_int_if_exists(int& ret_val) const;

  /**
   * \brief Returns the value as a double.
   *
   * If the entry exists and can be represented as a double, the value is passed to
   * the caller through the parameter and \c true is returned.  If the entry exists
   * but is not representable as a double, an IncompatibleConversion exception is
   * thrown.  If the entry does not exist, \c false is returned and the value of
   * the parameter is not changed.
   *
   * \param ret_val The parameter through which the value is returned.
   *
   * \return \c true if the value exists, \c false otherwise.
   */
  bool as_double_if_exists(double& ret_val) const;

  /**
   * \brief Returns the value as a string.
   *
   * If the entry exists and can be represented as a string, the value is passed to
   * the caller through the parameter and \c true is returned.  If the entry exists
   * but is not representable as a string, an IncompatibleConversion exception is
   * thrown.  If the entry does not exist, \c false is returned and the value of
   * the parameter is not changed.
   *
   * \param ret_val The parameter through which the value is returned.
   *
   * \return \c true if the value exists, \c false otherwise.
   */
  bool as_string_if_exists(std::string& ret_val) const;

  /**
   * \brief Returns the value as an int tuple.
   *
   * If the entry exists and can be represented as an int tuple, the value is
   * passed to the caller through the parameter and \c true is returned.  If the
   * value exists but is not representable as an int tuple, an
   * IncompatibleConversion exception is thrown.  If the entry does not exist, \c
   * false is returned and the value of the parameter is not changed.
   *
   * \param ret_val The parameter through which the value is returned.
   *
   * \return \c true if the value exists, \c false otherwise.
   */
  bool as_int_tuple_if_exists(IntTuple& ret_val) const;

  /**
   * \brief Returns the value as a double tuple.
   *
   * If the entry exists and can be represented as a double tuple, the value is
   * passed to the caller through the parameter and \c true is returned.  If the
   * value exists but is not representable as a double tuple, an
   * IncompatibleConversion exception is thrown.  If the entry does not exist, \c
   * false is returned and the value of the parameter is not changed.
   *
   * \param ret_val The parameter through which the value is returned.
   *
   * \return \c true if the value exists, \c false otherwise.
   */
  bool as_double_tuple_if_exists(DoubleTuple& ret_val) const;

  /**
   * \brief Returns the value as an int.
   *
   * If the entry exists and can be represented as an int, it is returned.  If the
   * value is not representable as an int, an IncompatibleConversion exception is
   * thrown.  If the entry does not exist, a NonexistentEntry exception is thrown.
   *
   * \return The value as an int.
   */
  int as_int_or_die() const;

  /**
   * \brief Returns the value as a double.
   *
   * If the entry exists and can be represented as a double, it is returned.  If
   * the value is not representable as a double, an IncompatibleConversion
   * exception is thrown.  If the entry does not exist, a NonexistentEntry
   * exception is thrown.
   *
   * \return The value as a double.
   */
  double as_double_or_die() const;

  /**
   * \brief Returns the value as a string.
   *
   * If the entry exists and can be represented as a string, it is returned.  If
   * the value is not representable as a string, an IncompatibleConversion
   * exception is thrown.  If the entry does not exist, a NonexistentEntry
   * exception is thrown.
   *
   * \return The value as a string.
   */
  std::string as_string_or_die() const;

  /**
   * \brief Returns the value as an int tuple.
   *
   * If the entry exists and can be represented as an int tuple, it is returned.
   * If the value is not representable as an int tuple, an IncompatibleConversion
   * exception is thrown.  If the entry does not exist, a NonexistentEntry
   * exception is thrown.
   *
   * \return The value as an int tuple.
   */
  IntTuple as_int_tuple_or_die() const;

  /**
   * \brief Returns the value as a double tuple.
   *
   * If the entry exists and can be represented as a double tuple, it is returned.
   * If the value is not representable as a double tuple, an IncompatibleConversion
   * exception is thrown.  If the entry does not exist, a NonexistentEntry
   * exception is thrown.
   *
   * \return The value as a double tuple.
   */
  DoubleTuple as_double_tuple_or_die() const;

  /**
   * \brief Returns the value as an int.
   *
   * If the entry exists and can be represented as an int, it is returned.  If the
   * value is not representable as an int, an IncompatibleConversion exception is
   * thrown.  If the entry does not exist, a default value is returned.
   *
   * \param def_val The default value that is returned if the value does not exist.
   *
   * \return The value as an int or the default value if the value does not exist.
   */
  int as_int_or_default(const int def_val) const;

  /**
   * \brief Returns the value as a double.
   *
   * If the entry exists and can be represented as a double, it is returned.  If
   * the value is not representable as a double, an IncompatibleConversion
   * exception is thrown.  If the entry does not exist, a default value is
   * returned.
   *
   * \param def_val The default value that is returned if the value does not exist.
   *
   * \return The value as a double or the default value if the value does not exist.
   */
  double as_double_or_default(const double def_val) const;

  /**
   * \brief Returns the value as a string.
   *
   * If the entry exists and can be represented as a string, it is returned.  If
   * the value is not representable as a string, an IncompatibleConversion
   * exception is thrown.  If the entry does not exist, a default value is
   * returned.
   *
   * \param def_val The default value that is returned if the value does not exist.
   *
   * \return The value as a string or the default value if the value does not exist.
   */
  std::string as_string_or_default(const std::string& def_val) const;

  /**
   * \brief Returns the value as an int tuple.
   *
   * If the entry exists and can be represented as an int tuple, it is returned.
   * If the value is not representable as an int tuple, an IncompatibleConversion
   * exception is thrown.  If the entry does not exist, a default value is
   * returned.
   *
   * \param def_val The default value that is returned if the value does not exist.
   *
   * \return The value as an int tuple or the default value if the value does not exist.
   */
  IntTuple as_int_tuple_or_default(const IntTuple& def_val) const;

  /**
   * \brief Returns the value as a double tuple.
   *
   * If the entry exists and can be represented as a double tuple, it is returned.
   * If the value is not representable as a double tuple, an IncompatibleConversion
   * exception is thrown.  If the entry does not exist, a default value is
   * returned.
   *
   * \param def_val The default value that is returned if the value does not exist.
   *
   * \return The value as a double tuple or the default value if the value does not exist.
   */
  DoubleTuple as_double_tuple_or_default(const DoubleTuple& def_val) const;

  /**
   * \brief An alias for as_int_or_die.
   *
   * This conversion operator allows a Value to be converted to an int when it is
   * assigned to an int variable.
   *
   * \return The int value of the Value.
   */
  operator int() const;

  /**
   * \brief An alias for as_double_or_die.
   *
   * This conversion operator allows a Value to be converted to a double when it is
   * assigned to a double variable.
   *
   * \return The double value of the Value.
   */
  operator double() const;

  /**
   * \brief An alias for as_string_or_die.
   *
   * This conversion operator allows a Value to be converted to a string when it is
   * assigned to a string variable.
   *
   * \return The string value of the Value.
   */
  operator std::string() const;

  /**
   * \brief An alias for as_int_tuple_or_die.
   *
   * This conversion operator allows a Value to be converted to an int tuple when
   * it is assigned to an int tuple variable.
   *
   * \return The int tuple value of the Value.
   */
  operator IntTuple() const;

  /**
   * \brief An alias for as_int_or_die.
   *
   * This conversion operator allows a Value to be converted to a double tuple when
   * it is assigned to a double tuple variable.
   *
   * \return The double tuple value of the Value.
   */
  operator DoubleTuple() const;

  /**
   * \brief An alias for as_int_or_default.
   *
   * \param def_val The value that is returned if the requested value does not exist.
   *
   * \return The requested value as an int or def_val if the value does not exist.
   */
  int operator||(const int def_val) const;

  /**
   * \brief An alias for as_double_or_default.
   *
   * \param def_val The value that is returned if the requested value does not exist.
   *
   * \return The requested value as a double or def_val if the value does not exist.
   */
  double operator||(const double def_val) const;

  /**
   * \brief An alias for as_string_or_default.
   *
   * \param def_val The value that is returned if the requested value does not exist.
   *
   * \return The requested value as a string or def_val if the value does not exist.
   */
  std::string operator||(const std::string& def_val) const;

  /**
   * \brief An alias for as_int_tuple_or_default.
   *
   * \param def_val The value that is returned if the requested value does not exist.
   *
   * \return The requested value as an int tuple or def_val if the value does not exist.
   */
  IntTuple operator||(const IntTuple& def_val) const;

  /**
   * \brief An alias for as_double_tuple_or_default.
   *
   * \param def_val The value that is returned if the requested value does not exist.
   *
   * \return The requested value as a double tuple or def_val if the value does not exist.
   */
  DoubleTuple operator||(const DoubleTuple& def_val) const;
};

/**
 * \brief The type of the map in which the values are stored.
 */
using MTLValueMap = std::unordered_map<std::string, MTLValue*>;

/**
 * \brief The type of the iterator for iterating over a ValueMap.
 */
using MTLValueIter = MTLValueMap::iterator;

/**
 * \brief The type of the iterator for iterating over a constant ValueMap.
 */
using ConstMTLValueIter = MTLValueMap::const_iterator;

/**
 * \brief The type that is used to represent materials that are stored in the mtl file.
 */
class Material
{
  private:
  /**
   * \brief The name of this material.
   */
  std::string material_name;

  /**
   * \brief A pointer to the map that stores the entries of the material.
   */
  const MTLValueMap* values;

  public:
  /**
   * \brief Creates a new material.
   *
   * \param material_name_init The name of the material.
   * \param values_init An iterator to the map that stores the entries of the material.
   */
  Material(const std::string& material_name_init, const MTLValueMap* const values_init);

  /**
   * \brief Creates a new material by copying another one.
   *
   * \param original The material that is copied.
   */
  Material(const Material& original);

  /**
   * \brief Destructs a section.
   */
  ~Material();

  /**
   * \brief Copies another material.
   *
   * \param original The material that is copied.
   *
   * \return A reference to this section.
   */
  Material& operator=(const Material& original);

  /**
   * \brief Looks up a entry in the material given its key and returns it.
   *
   * \param key The entry corresponding to the key.
   *
   * \return The entry corresponding to the key or an empty entry if the requested entry does not exist.
   */
  MTLEntry operator[](const std::string& key) const;
};

/**
 * \brief The type in which a material library is stored.
 */
class MTLLibrary
{
  private:
  /**
   * \brief The type of the map in which materials are stored.
   */
  using MaterialMap = std::unordered_map<std::string, MTLValueMap>;

  /**
   * \brief The iterator for iterating over a MaterialMap.
   */
  using SectionIter = MaterialMap::iterator;

  /**
   * \brief The iterator for iterating over a constant MaterialMap.
   */
  using ConstMaterialIter = MaterialMap::const_iterator;

  /**
   * \brief Maps the names of the materials on the entries in them.
   */
  MaterialMap materials;

  /**
   * \brief Deletes the entries in a material.
   */
  static void delete_material(const MaterialMap::value_type& material);

  /**
   * \brief Constructs an MTL library by copying another one.
   *
   * This copy-assignment operator is made private in order to avoid copying.
   * Material libraries should not be copied, they should be passed by reference
   * instead.
   *
   * \param original The MTL library that is copied.
   */
  MTLLibrary(const MTLLibrary& original);

  /**
   * \brief Copies an MTL library.
   *
   * This copy-assignment operator is made private in order to avoid copying.
   * Material libraries should not be copied, they should be passed by reference
   * instead.
   *
   * \param original The MTL library that is copied.
   *
   * \return A reference to this MTLLibrary.
   */
  MTLLibrary& operator=(const MTLLibrary& original);

  public:
  /**
   * \brief Constructs a new (empty) material library.
   */
  MTLLibrary();

  /**
   * \brief Constructs a new MTLLibrary by parsing the content from a stream.
   *
   * \param input_stream The stream from which the content is parsed.
   */
  MTLLibrary(std::istream& input_stream);

  /**
   * \brief Destructs a MTLLibrary and frees all entries stored in it.
   */
  ~MTLLibrary();

  /**
   * \brief Retrieves a section from the mtl library file given its key.
   *
   * If the requested section does not exist, a material containing no values is
   * returned.
   *
   * \param key The name of the requested material.
   *
   * \return A reference to the requested material.
   */
  Material operator[](const std::string& key) const;

  /**
   * \brief Reads a mtl library file from a stream.
   *
   * \param input_stream The input stream from which the mtl library is read.
   */
  void parse(std::istream& input_stream);

  /**
   * \brief Formats the contents of the MTLLibrary to text and prints it to an output stream.
   *
   * \param output_stream The output stream to which the output is written.
   */
  void print(std::ostream& output_stream) const;
};

/**
 * \brief Convenience operator for reading mtl libraries from an input stream.
 *
 * This operator reads the mtl library from the input stream using the
 * MTLLibrary::parse method.
 *
 * \param input_stream  The input stream from which the mtl library is read.
 * \param mtllib The MTLLibrary object in which the parsed mtl library is stored.
 *
 * \return A reference to the input stream.
 */
std::istream& operator>>(std::istream& input_stream, MTLLibrary& mtllib);

/**
 * \brief Convenience operator for writing mtl libraries to an output stream.
 *
 * This operator prints the mtl library to the output stream using the
 * MTLLibrary::print method.
 *
 * \param output_stream The output stream to which the mtl library is written.
 * \param mtllib The MTLLibrary object in which the parsed mtl library is stored.
 *
 * \return A reference to the output stream.
 */
std::ostream& operator<<(std::ostream& output_stream, const MTLLibrary& mtllib);

/**
 * \brief The type in which an object (group) is stored.
 */
class ObjectGroup
{

  private:
  /**
   * \brief mtl_name ID of material
   */
  std::string mtl_name;

  /**
   * \brief mtl_lib filename of material library used
   */
  std::string mtllib_filename;

  /**
   * \brief A vector of all vertexes.
   */
  std::vector<DoubleTuple> vertexes;

  /**
   * \brief A vector of all texture coordinates.
   */
  std::vector<DoubleTuple> texture_coordinates;

  /**
   * \brief A vector of all vertex normals.
   */
  std::vector<DoubleTuple> vertex_normals;

  /**
   * \brief A vector of all polygonal faces.
   */
  std::vector<Polygon> polygons;

  /**
   * \brief Parse a line containing a vertex.
   *
   * \param tokens Whitespace separated tokens.
   *
   * \param line Complete line.
   */
  void parse_vertex(const std::vector<std::string>& tokens, const std::string& line);

  /**
   * \brief Parse a line containing a vertex normal.
   *
   * \param tokens Whitespace separated tokens.
   *
   * \param line Complete line.
   */
  void parse_vertex_normal(const std::vector<std::string>& tokens, const std::string& line);

  /**
   * \brief Parse a line containing texture coordinates.
   *
   * \param tokens Whitespace separated tokens.
   *
   * \param line Complete line.
   */
  void parse_texture_coordinates(const std::vector<std::string>& tokens, const std::string& line);

  /**
   * \brief Parse a line containing a polygon.
   *
   * \param tokens Whitespace separated tokens.
   *
   * \param line Complete line.
   */
  void parse_polygon(const std::vector<std::string>& tokens, const std::string& line);

  /**
   * \brief Parse a line containing a mtllib declaration.
   *
   * \param tokens Whitespace separated tokens.
   *
   * \param line Complete line.
   */
  void parse_mtllib(const std::vector<std::string>& tokens, const std::string& line);

  /**
   * \brief Parse a line containing a usemtl declaration.
   *
   * \param tokens Whitespace separated tokens.
   *
   * \param line Complete line.
   */
  void parse_usemtl(const std::vector<std::string>& tokens, const std::string& line);

  /**
   * \brief Parse a line of the obj file.
   *
   * \param line Complete line.
   */
  void parse_obj_line(const std::string& line);

  public:
  /**
   * \brief Constructs a new (empty) mesh.
   */
  ObjectGroup() = default;

  /**
   * \brief Constructs a new Mesh by parsing the content from a stream.
   *
   * \param input_stream The stream from which the content is parsed.
   */
  ObjectGroup(std::istream& input_stream);

  /**
   * \brief Destructs a MTLLibrary and frees all entries stored in it.
   */
  ~ObjectGroup() = default;

  /**
   * \brief Retrieves the vertex coordinates for a mesh.
   *
   * \return Vector of vertex coordinates in {x,y,z[,w]} format. w is optional.
   */
  std::vector<DoubleTuple> get_vertexes();

  /**
   * \brief Retrieves the vertex coordinates for a mesh.
   *
   * \return Vector of vertex coordinates in {x,y,z[,w]} format. w is optional.
   */
  const std::vector<DoubleTuple>& get_vertexes() const;

  /**
   * \brief Retrieves the texture coordinates for a mesh.
   *
   * \return Vector of texture coordinates in {u[,v,w]} format. v and w are optional.
   */
  std::vector<DoubleTuple> get_texture_coordinates();

  /**
   * \brief Retrieves the texture coordinates for a mesh.
   *
   * \return Vector of texture coordinates in {u[,v,w]} format. v and w are optional.
   */
  const std::vector<DoubleTuple>& get_texture_coordinates() const;

  /**
   * \brief Retrieves the vertex normals from the obj file.
   *
   * \return Vector containing all vertex normals in {x,y,z} form. These might not be unit vectors.
   */
  std::vector<DoubleTuple> get_vertex_normals();

  /**
   * \brief Retrieves the vertex normals from the obj file.
   *
   * \return Vector containing all vertex normals in {x,y,z} form. These might not be unit vectors.
   */
  const std::vector<DoubleTuple>& get_vertex_normals() const;

  /**
   * \brief Retrieves the polygons from the obj file.
   *
   * \return Vector containing all polygonal faces.
   */
  std::vector<Polygon> get_polygons();

  /**
   * \brief Retrieves the polygons from the obj file.
   *
   * \return Vector containing all polygonal faces.
   */
  const std::vector<Polygon>& get_polygons() const;

  /**
   * \brief Retrieves the mtl name used for this group.
   *
   * \return The name of the mtl used for this group.
   */
  std::string get_mtl_name();

  /**
   * \brief Retrieves the mtl name used for this group.
   *
   * \return The name of the mtl used for this group.
   */
  const std::string& get_mtl_name() const;

  /**
   * \brief Retrieves the mtl name used for this group.
   *
   * \return The name of the mtl used for this group.
   */
  std::string get_mtllib_file_name();

  /**
   * \brief Retrieves the mtllib file name used for this group.
   *
   * \return The name of the mtllib file used for this group.
   */
  const std::string& get_mtllib_file_name() const;

  /**
   * \brief Reads an obj file from a stream.
   *
   * \param input_stream The input stream from which the mesh is read.
   */
  void parse(std::istream& input_stream);

  /**
   * \brief Formats the contents of the mesh to text and prints it to an output stream.
   *
   * \param output_stream The output stream to which the output is written.
   */
  void print(std::ostream& output_stream) const;
};

/**
 * \brief Convenience operator for writing groups to an output stream.
 *
 * This operator prints the mesh to the output stream using the
 * ObjectGroup::print method.
 *
 * \param output_stream The output stream to which the group is written.
 * \param mesh The ObjectGroup object in which the parsed group is stored.
 *
 * \return A reference to the output stream.
 */
std::ostream& operator<<(std::ostream& output_stream, const ObjectGroup& mesh);

/**
 * \brief The type in which an obj file is stored.
 */
class OBJFile
{

  private:
  /**
   * \brief Object group that belongs to this OBJ file
   */
  ObjectGroup object;

  /**
   * \brief Constructs a mesh by copying another one.
   *
   * This copy-assignment operator is made private in order to avoid copying.
   *  Meshes should not be copied, they should be passed by reference
   * instead.
   *
   * \param original The mesh that is copied.
   */
  OBJFile(const OBJFile& original);

  /**
   * \brief Copies a mesh.
   *
   * This copy-assignment operator is made private in order to avoid copying.
   * Meshes should not be copied, they should be passed by reference
   * instead.
   *
   * \param original The mesh that is copied.
   *
   * \return A reference to this mesh.
   */
  OBJFile& operator=(const OBJFile& original);

  public:
  /**
   * \brief Constructs a new (empty) mesh.
   */
  OBJFile() = default;

  /**
   * \brief Constructs a new Mesh by parsing the content from a stream.
   *
   * \param input_stream The stream from which the content is parsed.
   */
  OBJFile(std::istream& input_stream);

  /**
   * \brief Destructs a MTLLibrary and frees all entries stored in it.
   */
  ~OBJFile() = default;

  /**
   * \brief Retrieves object in this file.
   *
   * \return Vector of object.
   */
  ObjectGroup get_object();

  /**
   * \brief Retrieves object in this file.
   *
   * \return Vector of object.
   */
  const ObjectGroup& get_object() const;

  /**
   * \brief Reads an obj file from a stream.
   *
   * \param input_stream The input stream from which the mesh is read.
   */
  void parse(std::istream& input_stream);

  /**
   * \brief Formats the contents of the mesh to text and prints it to an output stream.
   *
   * \param output_stream The output stream to which the output is written.
   */
  void print(std::ostream& output_stream) const;
};

/**
 * \brief Convenience operator for reading files from an input stream.
 *
 * This operator reads the file from the input stream using the
 * OBJFile::parse method.
 *
 * \param input_stream  The input stream from which the file is read.
 * \param mesh The OBJFile object in which the parsed file is stored.
 *
 * \return A reference to the input stream.
 */
std::istream& operator>>(std::istream& input_stream, OBJFile& mesh);

/**
 * \brief Convenience operator for writing files to an output stream.
 *
 * This operator prints the file to the output stream using the
 * OBJFile::print method.
 *
 * \param output_stream The output stream to which the file is written.
 * \param mesh The OBJFile object in which the parsed file is stored.
 *
 * \return A reference to the output stream.
 */
std::ostream& operator<<(std::ostream& output_stream, const OBJFile& mesh);
} // namespace obj

#endif // OBJ_PARSER_INCLUDED
