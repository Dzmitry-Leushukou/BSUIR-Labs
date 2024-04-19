extern "C" __declspec(dllexport) long long check(long long *a, long long l, long long r)
{
	if (l == r)
	{
		long long q = a[l] * a[l] * a[l];
		return (q <= -10) + (q >= 20);
	}
	long long mid = (l + r) / 2;
	return check(a, l, mid) + check(a, mid + 1, r);
}