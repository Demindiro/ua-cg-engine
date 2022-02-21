#pragma once

class TypeException : public std::exception {
	std::string type;
	
public:
	TypeException(const std::string type) throw();

	virtual const char *what() const throw();
};
