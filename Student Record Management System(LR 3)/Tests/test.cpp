#include "pch.h"
#include "Application.h"
#include "Checker.h"
#include "Student.h"

TEST(API, ServerNotWork)
{
	EXPECT_THROW(API::get(), std::runtime_error);
}
TEST(Application, GetStudentDataWithIncorID)
{
	Application* ap = new Application();
	EXPECT_THROW(ap->getStudentData(-1), std::invalid_argument);
}
TEST(Checker, CheckID_Div)
{
	EXPECT_EQ(Checker::checkId("1.0"), 0);
}
TEST(Checker, CheckID_Big)
{
	EXPECT_EQ(Checker::checkId("100000000000000000000000000"), 0);
}
TEST(Checker, CheckID_Small)
{
	EXPECT_EQ(Checker::checkId("-100000000000000000000000000"), 0);
}
TEST(Checker, CheckID_Char)
{
	EXPECT_EQ(Checker::checkId("q"), 0);
}
TEST(Student, StudentConstructor1)
{
	EXPECT_THROW(Student(0,"", {","}), std::invalid_argument);
}
TEST(Student, StudentConstructor2)
{
	EXPECT_NO_THROW(Student(0, "", { "1," }));
}
TEST(Student, StudentConstructor3)
{
	EXPECT_THROW(Student(0, "", { ",2" }), std::invalid_argument);
}
TEST(Student, StudentConstructor4)
{
	EXPECT_NO_THROW(Student(0, "", { "1" }));
}
TEST(Student, StudentConstructor5)
{
	EXPECT_NO_THROW(Student(0, "", { "" }));
}
TEST(Student, StudentConstructor6)
{
	EXPECT_THROW(Student(0, "", { " " }), std::invalid_argument);
}
TEST(Student, StudentConstructor7)
{
	EXPECT_THROW(Student(0, "", { "lol" }), std::invalid_argument);
}
TEST(Student, StudentConstructor8)
{
	EXPECT_THROW(Student(0, "", { "1.0" }), std::invalid_argument);
}
