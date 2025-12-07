namespace BattleshipGame.Shared.Models
{
    public class Shot
    {
        public int X { get; set; }
        public int Y { get; set; }
        public bool IsHit { get; set; }
        public DateTime Timestamp { get; set; } = DateTime.UtcNow;
        public string? PlayerConnectionId { get; set; }

        public Shot() { }

        public Shot(int x, int y)
        {
            X = x;
            Y = y;
        }
    }
}