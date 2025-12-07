using Microsoft.AspNetCore.Mvc;
using BattleshipGame.Server.Services;
using BattleshipGame.Shared.Models;

namespace BattleshipGame.Server.Controllers
{
    [ApiController]
    [Route("api/[controller]")]
    public class GameController : ControllerBase
    {
        private readonly GameManager _gameManager;
        private readonly ILogger<GameController> _logger;

        public GameController(GameManager gameManager, ILogger<GameController> logger)
        {
            _gameManager = gameManager;
            _logger = logger;
        }

        [HttpGet("active")]
        public IActionResult GetActiveGames()
        {
            var games = _gameManager.GetActiveSessions();
            return Ok(games);
        }

        [HttpGet("session/{sessionId}")]
        public IActionResult GetSession(string sessionId)
        {
            var session = _gameManager.GetSession(sessionId);
            if (session == null)
                return NotFound();

            return Ok(session);
        }

        [HttpPost("create")]
        public IActionResult CreateGame([FromBody] CreateGameRequest request)
        {
            var session = _gameManager.CreateNewGame();
            return Ok(new { sessionId = session.SessionId });
        }

        [HttpGet("health")]
        public IActionResult HealthCheck()
        {
            return Ok(new { status = "Server is running", timestamp = DateTime.UtcNow });
        }
    }

    public class CreateGameRequest
    {
        public string? PlayerName { get; set; }
    }
}