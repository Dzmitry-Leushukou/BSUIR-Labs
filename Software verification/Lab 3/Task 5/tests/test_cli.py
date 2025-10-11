
import unittest
from io import StringIO
import sys
from pathlib import Path
from tempfile import TemporaryDirectory

from file_finder.cli import main


class TestCLI(unittest.TestCase):
    def run_cmd(self, args):
        saved_out, saved_err = sys.stdout, sys.stderr
        try:
            out, err = StringIO(), StringIO()
            sys.stdout, sys.stderr = out, err
            code = main(args)
            return code, out.getvalue(), err.getvalue()
        finally:
            sys.stdout, sys.stderr = saved_out, saved_err

    def test_cli_outputs_paths(self):
        with TemporaryDirectory() as d:
            base = Path(d)
            (base / "x").mkdir()
            (base / "x" / "a.txt").write_text("1", encoding="utf-8")
            (base / "x" / "b.TXT").write_text("2", encoding="utf-8")
            (base / "x" / "c.md").write_text("3", encoding="utf-8")

            code, out, err = self.run_cmd([str(base), "txt"])
            self.assertEqual(code, 0)
            lines = out.strip().splitlines()
            self.assertEqual(len(lines), 2)
            self.assertTrue(lines[0].endswith("/a.txt"))
            self.assertTrue(lines[1].endswith("/b.TXT"))
            self.assertEqual(err, "")  # nothing found would go to stderr

    def test_cli_error(self):
        code, out, err = self.run_cmd(["/path/not/exist", "txt"])
        self.assertNotEqual(code, 0)
        self.assertIn("Ошибка:", err)


if __name__ == "__main__":
    unittest.main()
