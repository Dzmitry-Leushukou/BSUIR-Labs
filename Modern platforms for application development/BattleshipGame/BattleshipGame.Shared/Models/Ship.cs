namespace BattleshipGame.Shared.Models
{
    public class Ship
    {
        public int Id { get; set; }
        public string Name { get; set; } = string.Empty;
        public int Size { get; set; }
        public List<Cell> Cells { get; set; } = new();
        public bool IsHorizontal { get; set; } = true;
        public bool IsPlaced { get; set; }

        public bool IsDestroyed => Cells.All(c => c.IsHit);

        public Ship() { }

        public Ship(int id, string name, int size)
        {
            Id = id;
            Name = name;
            Size = size;
        }
    }
}