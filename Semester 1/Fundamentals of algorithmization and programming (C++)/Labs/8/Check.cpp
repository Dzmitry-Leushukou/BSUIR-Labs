#include <iostream>
#include <string>
#include <fstream>

#define ll long long
#define ld long double
#define SEEK_SET 0    // ������������ ������ �����
// ������ ����� - ������� 0
#define  SEEK_CUR 1    // ������������ ������� �������,
// >0 - ������, <0 - �����
#define SEEK_END 2    // ������������ ����� �����
// (�������� pos - �������������)
int main()
{
std::fstream file;
file.open("data.bin", std::ios::out);
file.close();
struct t{

ll a;
ll b;
};
t Koran={5,6};
file.open("data.bin", std::ios::binary | std::ios::app);
	file.write((char*)&Koran, sizeof(t));
	file.close();
file.open("data.bin", std::ios_base::binary | std::ios_base::in);
ll size=1;
		for (ll i = 0; i < size; i++)
		{
			t Koran;
			file.read((char*)&Koran, sizeof(Koran));
			std::cout<<Koran.a<<" "<<Koran.b;
			std::cout << '\n';
		}
		file.close();
}
