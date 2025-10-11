
import io
import os
import unittest
from unittest import mock

from url_fetcher.downloader import fetch, DownloadError


class FakeResponse:
    def __init__(self, content: bytes, headers: dict, url: str):
        self._content = io.BytesIO(content)
        self.headers = headers
        self._url = url

    def read(self, n=-1):
        return self._content.read(n)

    def geturl(self):
        return self._url

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc, tb):
        return False


class TestDownloader(unittest.TestCase):
    @mock.patch("urllib.request.urlopen")
    def test_download_ok_with_cd(self, m_urlopen):
        content = b"hello world"
        headers = {"Content-Disposition": 'attachment; filename="hello.txt"'}
        m_urlopen.return_value = FakeResponse(content, headers, "https://example.com/download")
        import tempfile
        with tempfile.TemporaryDirectory() as d:
            path = fetch("https://example.com/download", d, timeout=1, retries=0)
            self.assertTrue(os.path.exists(path))
            self.assertTrue(path.endswith("hello.txt"))
            with open(path, "rb") as f:
                self.assertEqual(f.read(), content)

    @mock.patch("urllib.request.urlopen")
    def test_download_ok_infer_name(self, m_urlopen):
        content = b"data"
        headers = {}
        m_urlopen.return_value = FakeResponse(content, headers, "https://example.com/path/file.bin")
        import tempfile
        with tempfile.TemporaryDirectory() as d:
            path = fetch("https://example.com/path/file.bin", d, timeout=1, retries=0)
            self.assertTrue(path.endswith("file.bin"))

    @mock.patch("urllib.request.urlopen")
    def test_retry_on_500_then_success(self, m_urlopen):
        from urllib.error import HTTPError
        def side_effect(*args, **kwargs):
            if not hasattr(side_effect, "called"):
                side_effect.called = True
                raise HTTPError(url="http://x", code=500, msg="err", hdrs=None, fp=None)
            return FakeResponse(b"x", {}, "http://x/file")
        m_urlopen.side_effect = side_effect
        import tempfile
        with tempfile.TemporaryDirectory() as d:
            path = fetch("http://x/file", d, timeout=1, retries=1)
            self.assertTrue(os.path.exists(path))

    def test_invalid_scheme(self):
        import tempfile
        with tempfile.TemporaryDirectory() as d:
            with self.assertRaises(DownloadError):
                fetch("ftp://example.com/a", d)

if __name__ == "__main__":
    unittest.main()
