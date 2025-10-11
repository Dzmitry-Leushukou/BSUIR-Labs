
import unittest
from io import StringIO
import sys

from rect_area.cli import main


class TestCLI(unittest.TestCase):
    def run_cmd(self, args):
        saved_out, saved_err = sys.stdout, sys.stderr
        try:
            out, err = StringIO(), StringIO()
            sys.stdout, sys.stderr = out, err
            code = main(args)
            return code, out.getvalue().strip(), err.getvalue().strip()
        finally:
            sys.stdout, sys.stderr = saved_out, saved_err

    def test_ok_ints(self):
        code, out, err = self.run_cmd(["3", "4"])
        self.assertEqual(code, 0)
        self.assertEqual(out, "12")
        self.assertEqual(err, "")

    def test_ok_decimals_and_precision(self):
        code, out, err = self.run_cmd(["2.5", "4", "--precision", "3"])
        self.assertEqual(code, 0)
        self.assertEqual(out, "10")
        self.assertEqual(err, "")

    def test_comma_decimal(self):
        code, out, err = self.run_cmd(["2,5", "4"])
        self.assertEqual(code, 0)
        self.assertEqual(out, "10")
        self.assertEqual(err, "")

    def test_error_negative(self):
        code, out, err = self.run_cmd(["-1", "2"])
        self.assertNotEqual(code, 0)
        self.assertIn("Ошибка:", err)

    def test_error_non_number(self):
        code, out, err = self.run_cmd(["abc", "2"])
        self.assertNotEqual(code, 0)
        self.assertIn("Ошибка:", err)


if __name__ == "__main__":
    unittest.main()
