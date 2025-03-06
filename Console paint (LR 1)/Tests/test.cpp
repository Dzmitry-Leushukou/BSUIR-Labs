#include "pch.h"
#include "../UI.h"

int readInt(std::string s, int& i, int line)
{
	std::string ch;
	while (s.at(i) >= '0' && s.at(i) <= '9')
	{
		ch += s.at(i);
		i++;
	}
	if (ch.size() == 0 && s.at(i) != ' ')
		throw std::invalid_argument("Wrong info line:" + std::to_string(line));
	i++;
	return stoi(ch);
}

TEST(ActionsClassTest, EmptyUndo) 
{
	Actions actions;
	EXPECT_THROW({ actions.undo(); }, std::invalid_argument);
}

TEST(ActionsClassTest, EmptyRedo)
{
	Actions actions;
	EXPECT_THROW({ actions.redo(); }, std::invalid_argument);
}

TEST(ActionsClassTest, NegativeUndo)
{
	Canvas* canvas = new Canvas(20, 60);
	Actions actions;
	EXPECT_THROW({ actions.addAction(canvas); actions.addAction(canvas); actions.undo(); actions.undo();}, std::invalid_argument);
}

TEST(ActionsClassTest, OFRedo1)
{
	Canvas* canvas = new Canvas(20, 60);
	Actions actions;
	EXPECT_THROW({ actions.addAction(canvas); actions.redo();}, std::invalid_argument);
}

TEST(ActionsClassTest, OFRedo2)
{
	Canvas* canvas = new Canvas(20, 60);
	Actions actions;
	EXPECT_THROW({ actions.addAction(canvas); actions.addAction(canvas); actions.undo(); actions.redo(); actions.redo(); }, std::invalid_argument);
}

TEST(CanvasClassTest, EmptyErase1)
{
	Canvas* canvas = new Canvas(20, 60);
	EXPECT_THROW({ canvas->erase(0); }, std::out_of_range);
}

TEST(CanvasClassTest, EmptyErase2)
{
	Canvas* canvas = new Canvas(20, 60);
	EXPECT_THROW({ canvas->erase(-1); }, std::out_of_range);
}

TEST(CanvasClassTest, DrawRectangle1)
{
	Canvas* canvas = new Canvas(20, 60);
	EXPECT_THROW({ canvas->draw(0, {{-1,0},{5,5}}); }, std::invalid_argument);
}

TEST(CanvasClassTest, DrawRectangle2)
{
	Canvas* canvas = new Canvas(20, 60);
	EXPECT_THROW({ canvas->draw(0, {{0,-1},{5,5}}); }, std::invalid_argument);
}

TEST(CanvasClassTest, DrawRectangle3)
{
	Canvas* canvas = new Canvas(20, 60);
	EXPECT_THROW({ canvas->draw(0, {{0,0},{20,5}}); }, std::invalid_argument);
}

TEST(CanvasClassTest, DrawRectangle4)
{
	Canvas* canvas = new Canvas(20, 60);
	EXPECT_THROW({ canvas->draw(0, {{0,0},{5,60}}); }, std::invalid_argument);
}

TEST(CanvasClassTest, DrawTriangle1)
{
	Canvas* canvas = new Canvas(20, 60);
	EXPECT_THROW({ canvas->draw(1, {{0,0},{5,5},{10,10}}); }, std::invalid_argument);
}

TEST(CanvasClassTest, DrawTriangle2)
{
	Canvas* canvas = new Canvas(20, 60);
	EXPECT_THROW({ canvas->draw(1, {{-1,0},{5,5},{10,10}}); }, std::invalid_argument);
}

TEST(CanvasClassTest, DrawTriangle3)
{
	Canvas* canvas = new Canvas(20, 60);
	EXPECT_THROW({ canvas->draw(1, {{0,-1},{5,5},{10,10}}); }, std::invalid_argument);
}

TEST(CanvasClassTest, DrawTriangle4)
{
	Canvas* canvas = new Canvas(20, 60);
	EXPECT_THROW({ canvas->draw(1, {{5,5},{5,5},{11,10}}); }, std::invalid_argument);
}

TEST(CanvasClassTest, DrawCircle1)
{
	Canvas* canvas = new Canvas(20, 60);
	EXPECT_THROW({ canvas->draw(2, {{0,0},{1,-1}}); }, std::invalid_argument);
}

TEST(CanvasClassTest, DrawCircle2)
{
	Canvas* canvas = new Canvas(20, 60);
	EXPECT_THROW({ canvas->draw(2, {{0,0},{1,0}}); }, std::invalid_argument);
}

TEST(CanvasClassTest, DrawCircle3)
{
	Canvas* canvas = new Canvas(20, 60);
	EXPECT_THROW({ canvas->draw(2, {{18,58},{4,0}}); }, std::invalid_argument);
}

TEST(SerializerClassTest, SerializeWithoutFile)
{
	char symb[3] = {  ' ',' ',' ' };
	std::vector<const Figure*>figures;

	EXPECT_THROW( Serializer::Serialize(symb,figures,"addasdds/"), std::invalid_argument);
}

TEST(SerializerClassTest, DeserializeWithoutFile)
{
	EXPECT_THROW(Serializer::Deserialize("addasdds/"), std::invalid_argument);
}

TEST(SerializerClassTest, DeserializeWithUnkownFile)
{
	EXPECT_THROW(Serializer::Deserialize("adddds.txt"), std::invalid_argument);
}

TEST(SerializerClassTest, DeserializeWithEmptyFile)
{
	EXPECT_THROW(Serializer::Deserialize("addasdds.txt"), std::invalid_argument);
}

TEST(SerializerClassTest, readInt_and_readNumberOFR)
{
	int i = 0;
	EXPECT_THROW(int a = readInt("",i,0), std::out_of_range);
}

TEST(SerializerClassTest, readInt_and_readNumberRU)
{
	int i = 0;
	EXPECT_THROW(int a = readInt("ÈˆÛˆ˚Ù‚Ù‡˚‡‡‡ˆ˝‡", i, 0), std::invalid_argument);
}

TEST(TriangleClassTest, TriagleConstWith2Equalsy)
{
	
	ASSERT_NO_THROW(new Triangle({ {5,5},{2,5},{0,0} }));
}

