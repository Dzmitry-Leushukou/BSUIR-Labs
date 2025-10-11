
import os
import unittest
import tempfile
from unittest import mock
from url_fetcher.cli import main


class TestCLI(unittest.TestCase):
    @mock.patch("urllib.request.urlopen")
    def test_cli_ok(self, m_urlopen):
        import io

        class Resp:
            def __init__(self):
                self.headers = {}
                self._b = io.BytesIO(b"abc")
            def geturl(self): return "https://example.com/a.txt"
            def read(self, n=-1): return self._b.read(n)
            def __enter__(self): return self
            def __exit__(self, exc_type, exc, tb): return False

        m_urlopen.return_value = Resp()

        with tempfile.TemporaryDirectory() as d:
            code = main(["https://example.com/a.txt", d])
            self.assertEqual(code, 0)
            self.assertTrue(any(name.endswith(".txt") for name in os.listdir(d)))

if __name__ == "__main__":
    unittest.main()
