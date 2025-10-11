
import unittest
from pathlib import Path
from tempfile import TemporaryDirectory
from file_finder.logic import find_files, normalize_extension, FinderError


class TestFinder(unittest.TestCase):
    def setUp(self):
        self.tmp = TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        base = Path(self.tmp.name)
        (base / "a").mkdir()
        (base / "a" / "b").mkdir()
        (base / "a" / "file1.txt").write_text("x", encoding="utf-8")
        (base / "a" / "b" / "file2.TXT").write_text("y", encoding="utf-8")
        (base / "a" / "b" / "file3.md").write_text("z", encoding="utf-8")
        self.base = base

    def test_normalize_extension(self):
        self.assertEqual(normalize_extension("txt", case_sensitive=False), ".txt")
        self.assertEqual(normalize_extension(".TXT", case_sensitive=False), ".txt")
        self.assertEqual(normalize_extension(".TXT", case_sensitive=True), ".TXT")

    def test_find_default_case_insensitive(self):
        files = find_files(self.base, "txt")
        # Порядок обхода: file1.txt -> file2.TXT
        self.assertEqual(len(files), 2)
        self.assertTrue(files[0].as_posix().endswith("/file1.txt"))
        self.assertTrue(files[1].as_posix().endswith("/file2.TXT"))

    def test_find_case_sensitive(self):
        files = find_files(self.base, "txt", case_sensitive=True)
        self.assertEqual(len(files), 1)
        self.assertTrue(files[0].as_posix().endswith("/file1.txt"))

    def test_nonexistent_dir(self):
        with self.assertRaises(FinderError):
            find_files(self.base / "nope", "txt")
