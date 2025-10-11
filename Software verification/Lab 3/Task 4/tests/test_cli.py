
import os
import unittest
import tempfile
from gradient_table.cli import main


class TestCLI(unittest.TestCase):
    def test_cli_writes_file(self):
        with tempfile.TemporaryDirectory() as tmp:
            out = os.path.join(tmp, "out.html")
            code = main([out, "--rows", "16", "--cols", "4", "--text", "{hex}"])
            self.assertEqual(code, 0)
            self.assertTrue(os.path.exists(out))
            with open(out, "r", encoding="utf-8") as f:
                data = f.read()
            self.assertIn("<table>", data)
            self.assertIn("#ffffff", data)
            self.assertIn("#000000", data)


if __name__ == "__main__":
    unittest.main()
