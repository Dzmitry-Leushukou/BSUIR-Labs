namespace BattleshipGame.Shared.Models
{
    public class Cell
    {
        public int X { get; set; }
        public int Y { get; set; }
        public bool HasShip { get; set; }
        public bool IsHit { get; set; }

        public Cell() { }

        public Cell(int x, int y)
        {
            X = x;
            Y = y;
        }
    }
}