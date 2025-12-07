namespace BattleshipGame.Shared.DTOs
{
    public class GameSettings
    {
        public int BoardSize { get; set; } = 10;
        public int MaxPlayers { get; set; } = 2;
        public bool AllowSpectators { get; set; } = false;
        public int TurnTimeLimitSeconds { get; set; } = 60;

        public static GameSettings Default => new GameSettings();
    }
}