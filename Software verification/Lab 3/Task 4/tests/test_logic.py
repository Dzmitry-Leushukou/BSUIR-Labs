
import unittest
from gradient_table.logic import grayscale_hex, generate_grayscale_values, generate_html_table


class TestLogic(unittest.TestCase):
    def test_grayscale_hex(self):
        self.assertEqual(grayscale_hex(255), "#ffffff")
        self.assertEqual(grayscale_hex(0), "#000000")
        self.assertEqual(grayscale_hex(17), "#111111")
        with self.assertRaises(ValueError):
            grayscale_hex(-1)
        with self.assertRaises(ValueError):
            grayscale_hex(256)

    def test_generate_grayscale_values_min_step(self):
        vals = generate_grayscale_values(256)
        self.assertEqual(vals[0], 255)
        self.assertEqual(vals[-1], 0)
        # Проверим монотонное убывание с шагом 1
        for a, b in zip(vals, vals[1:]):
            self.assertEqual(a - b, 1)
        self.assertEqual(set(vals), set(range(0,256)))

    def test_generate_grayscale_values_small(self):
        vals = generate_grayscale_values(4)
        self.assertEqual(vals, [255, 170, 85, 0])

    def test_generate_html(self):
        html = generate_html_table(rows=5, cols=3, cell_text="{hex}/{value}")
        self.assertIn("<table>", html)
        self.assertIn("#ffffff", html)
        self.assertIn("#000000", html)
        self.assertNotIn("{hex}", html)
        self.assertNotIn("{value}", html)


if __name__ == "__main__":
    unittest.main()
