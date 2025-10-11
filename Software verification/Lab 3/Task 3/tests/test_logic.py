
import unittest
from decimal import Decimal
from rect_area.logic import parse_nonnegative_number, compute_area, format_decimal, ParseNumberError


class TestLogic(unittest.TestCase):
    def test_parse_numbers(self):
        self.assertEqual(parse_nonnegative_number("3"), Decimal(3))
        self.assertEqual(parse_nonnegative_number("2.5"), Decimal("2.5"))
        self.assertEqual(parse_nonnegative_number("2,5"), Decimal("2.5"))
        self.assertEqual(parse_nonnegative_number("+4"), Decimal(4))
        self.assertEqual(parse_nonnegative_number("1e3"), Decimal("1000"))

    def test_parse_invalid(self):
        with self.assertRaises(ParseNumberError):
            parse_nonnegative_number("")
        with self.assertRaises(ParseNumberError):
            parse_nonnegative_number("-1")
        with self.assertRaises(ParseNumberError):
            parse_nonnegative_number("abc")

    def test_compute_area(self):
        a = parse_nonnegative_number("3")
        b = parse_nonnegative_number("4")
        self.assertEqual(compute_area(a, b), Decimal(12))

        a = parse_nonnegative_number("2.5")
        b = parse_nonnegative_number("4")
        self.assertEqual(compute_area(a, b), Decimal("10"))

    def test_format_decimal(self):
        self.assertEqual(format_decimal(Decimal("10.000")), "10")
        self.assertEqual(format_decimal(Decimal("10.500")), "10.5")
        self.assertEqual(format_decimal(Decimal("0")), "0")
        self.assertEqual(format_decimal(Decimal("1.2345"), places=2), "1.23")
        self.assertEqual(format_decimal(Decimal("1.2"), places=4), "1.2")

if __name__ == "__main__":
    unittest.main()
