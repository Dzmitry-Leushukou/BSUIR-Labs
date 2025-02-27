#pragma once
#ifndef T_FUNC_H
#define T_FUNC_H

T::operator==(T& b)
{
	return (name == b.name && kol == b.kol && price == b.price && dat == b.dat && expiration == b.expiration);
}

#endif