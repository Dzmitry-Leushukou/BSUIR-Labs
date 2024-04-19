#include<string>

#include "T.h"


bool T::operator==(T &b)
{
		return (name == b.name && kol == b.kol && price == b.price && dat == b.dat && expiration == b.expiration);
}
