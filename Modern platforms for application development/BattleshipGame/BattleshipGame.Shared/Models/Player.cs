namespace BattleshipGame.Shared.Models
{
    public class Player
    {
        public string ConnectionId { get; set; } = string.Empty;
        public string Name { get; set; } = string.Empty;
        public List<Ship> Ships { get; set; } = new();
        public List<Shot> ShotsFired { get; set; } = new();
        public List<Shot> ShotsReceived { get; set; } = new();
        public bool IsReady { get; set; }
        public bool HasPlacedShips { get; set; }
        public bool IsMyTurn { get; set; }

        public Player() { }

        public Player(string connectionId, string name)
        {
            ConnectionId = connectionId;
            Name = name;
        }

        // Стандартный набор кораблей для морского боя
        public static List<Ship> GetDefaultShips()
        {
            return new List<Ship>
            {
                new Ship(1, "Авианосец", 4),
                new Ship(2, "Линкор", 3),
                new Ship(3, "Крейсер", 3),
                new Ship(4, "Эсминец", 2),
                new Ship(5, "Эсминец", 2),
                new Ship(6, "Катер", 1),
                new Ship(7, "Катер", 1),
                new Ship(8, "Катер", 1),
                new Ship(9, "Катер", 1),
                new Ship(10, "Катер", 1)
            };
        }
    }
}