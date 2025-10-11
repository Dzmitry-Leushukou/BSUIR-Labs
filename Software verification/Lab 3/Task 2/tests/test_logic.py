import unittest
from people_app.logic import parse_person_line, compute_stats, ParseError, Person


class TestLogic(unittest.TestCase):
    def test_parse_ok(self):
        p = parse_person_line("Иван Иванов 20")
        self.assertEqual(p, Person(first_name="Иван", last_name="Иванов", age=20))

    def test_parse_strips_spaces(self):
        p = parse_person_line("  Anna   Petrova   35  ")
        self.assertEqual(p.first_name, "Anna")
        self.assertEqual(p.last_name, "Petrova")
        self.assertEqual(p.age, 35)

    def test_parse_errors(self):
        with self.assertRaises(ParseError):
            parse_person_line("")
        with self.assertRaises(ParseError):
            parse_person_line("Иванов 20")
        with self.assertRaises(ParseError):
            parse_person_line("Иван Иванов 20 40")
        with self.assertRaises(ParseError):
            parse_person_line("Иван Иванов двадцать")  # not int
        with self.assertRaises(ParseError):
            parse_person_line("Иван Иванов -1")  # negative
        with self.assertRaises(ParseError):
            parse_person_line("Ив@н Иванов 20")  # invalid name

    def test_compute_stats(self):
        mn, mx, avg = compute_stats([10, 20, 30])
        self.assertEqual(mn, 10)
        self.assertEqual(mx, 30)
        self.assertAlmostEqual(avg, 20.0, places=10)

    def test_compute_stats_empty(self):
        with self.assertRaises(ValueError):
            compute_stats([])


if __name__ == "__main__":
    unittest.main()