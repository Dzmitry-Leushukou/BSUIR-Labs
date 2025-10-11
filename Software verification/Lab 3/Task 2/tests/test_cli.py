import unittest
from io import StringIO
import sys

from people_app.cli import main


class TestCLI(unittest.TestCase):
    def run_with_input(self, input_text: str):
        saved_stdin = sys.stdin
        saved_stdout = sys.stdout
        saved_stderr = sys.stderr
        try:
            sys.stdin = StringIO(input_text)
            out = StringIO()
            err = StringIO()
            sys.stdout = out
            sys.stderr = err
            code = main([])
            return code, out.getvalue(), err.getvalue()
        finally:
            sys.stdin = saved_stdin
            sys.stdout = saved_stdout
            sys.stderr = saved_stderr

    def test_happy_path(self):
        code, out, err = self.run_with_input("Иван Иванов 18\nПетр Петров 30\n\n")
        self.assertEqual(code, 0)
        lines = out.strip().splitlines()
        self.assertEqual(lines[0], "Иванов Иван 18")
        self.assertEqual(lines[1], "Петров Петр 30")
        self.assertRegex(lines[2], r"^18 30 24\.00$")
        self.assertEqual(err, "")

    def test_no_people(self):
        code, out, err = self.run_with_input("\n")
        self.assertEqual(code, 1)
        self.assertEqual(out, "")
        self.assertIn("не введено ни одного человека", err)

    def test_invalid_input(self):
        code, out, err = self.run_with_input("Bad Line\n")
        self.assertEqual(code, 1)
        self.assertEqual(out, "")
        self.assertIn("Ошибка:", err)


if __name__ == "__main__":
    unittest.main()