
import os
import unittest
from url_fetcher.utils import safe_filename, dedupe_filename, is_writable_dir

class TestUtils(unittest.TestCase):
    def test_safe_filename(self):
        self.assertEqual(safe_filename("report:2025?.pdf"), "report_2025_.pdf")
        self.assertEqual(safe_filename("..env"), "_env")
        self.assertEqual(safe_filename(""), "download")

    def test_dedupe(self):
        import tempfile
        with tempfile.TemporaryDirectory() as d:
            base = "file.txt"
            p1 = os.path.join(d, base)
            with open(p1, "w") as f: f.write("x")
            cand = dedupe_filename(d, base)
            self.assertTrue(cand.startswith("file (1)"))

    def test_is_writable_dir(self):
        import tempfile
        with tempfile.TemporaryDirectory() as d:
            self.assertTrue(is_writable_dir(d))

if __name__ == "__main__":
    unittest.main()
