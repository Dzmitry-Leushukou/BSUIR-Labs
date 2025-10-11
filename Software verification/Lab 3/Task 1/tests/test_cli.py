
import unittest
from unittest.mock import patch
from io import StringIO
import sys

from hello_app.cli import main, build_lines


class TestHelloApp(unittest.TestCase):
    def run_main_capture(self):
        saved_out, saved_err = sys.stdout, sys.stderr
        try:
            out, err = StringIO(), StringIO()
            sys.stdout, sys.stderr = out, err
            code = main([])
            return code, out.getvalue(), err.getvalue()
        finally:
            sys.stdout, sys.stderr = saved_out, saved_err

    def test_text_lines_with_patch(self):
        with patch("hello_app.cli.random.randint", return_value=10):
            lines = build_lines()
        self.assertEqual(lines[0], "Hello, world!")
        self.assertEqual(lines[1], "And hi again!")
        self.assertEqual(lines[2], "!" * 10)

    def test_main_output_structure(self):
        code, out, err = self.run_main_capture()
        self.assertEqual(code, 0)
        lines = out.strip().splitlines()
        self.assertEqual(lines[0], "Hello, world!")
        self.assertEqual(lines[1], "And hi again!")
        self.assertTrue(set(lines[2]) <= {"!"})
        self.assertGreaterEqual(len(lines[2]), 5)
        self.assertLessEqual(len(lines[2]), 50)
        self.assertEqual(err, "")

    def test_invalid_count_raises(self):
        with self.assertRaises(ValueError):
            build_lines(4)
        with self.assertRaises(ValueError):
            build_lines(51)


if __name__ == "__main__":
    unittest.main()
