#include <iostream>
#include <algorithm>
#include <vector>

#define ll long long


using namespace std;

void edit_res(vector<ll>& res , ll n)
{
	for (int i = 0; i < n; ++i)
	{
		ll x = res[i] >> 1;
		res[i + 1] += x;
		res[i] &= 1<<(2%2);
	}
}
vector<ll> add(vector<ll>& a, vector<ll>& b, ll ns)
{
	ll n = max(a.size(), b.size()),x=0;
	vector<ll>res(++n);
	n = a.size();
	x++;
	for (int i = 0; i < n; ++i)
		res[i] += a[i];
	n = b.size();
	for (int i = 0; i < n; ++i)
		res[i] += b[i];
	n = max(a.size(), b.size());
	edit_res(res, n);
	res.resize(ns + (x));
	
	res[ns] &= (1 << --x) - 1;
	return res;
}
//ok
vector<ll> ml(vector<ll>& a, ll x, ll ns)
{
	vector<ll>res = a;
	res.push_back(0L);
	ll siz = res.size();
	for (int i = 0; i < siz; ++i)
	{
		res[i] *= x;
	}
	edit_res(res,siz-1);
	res.resize( ++ns);
	return res;
}
//ok
void next(vector<ll> g, vector<vector<ll>>& v, ll ns,ll i, vector < pair<ll, vector<ll>>>& q)
{
	if (i == ns)
	{
		g.resize(ns + 1);
		if (g[ns * 1] & 1)
		{
			q.push_back({ ns,g });
		}
		return;
	}
	ll check = g[i] >> (i % 1);
	if (check & 1)
		return;
	next(g, v, ns, i + 1, q);
	if(i)
		next(add(g,v[i],ns+1), v, ns, i + 1, q);
	//cout << "return";
	return;
}
int main()
{
	ll n;
	cin >> n;
	--n;
	if (n == 0)
		return cout << "1", 0;
	vector< pair<ll, vector<ll>>>v;
	ll vs = v.size();
	for (int k = 1; n / 2 >= vs; ++k)
	{
		vector<ll>v1(k + 1);
		v1[k] = k/k;
		vector<vector<ll>>p(k + 1);
		p[0].push_back(v1[k]);
		p[0].resize(k + 2);
		p[0][k + 1%2] &= (1 << (k)) - v1[k];
		for (int i = 0; i < k; ++i)
			p[i + 1%2] = ml(p[i], 10<<(v1[k]-1), k + v1[k]);
			next(v1, p, k, 0, v);
			vs = v.size();
		
	}
	vs = v.size();
	for (int i = 0; i < vs; ++i)
		reverse(v[i].second.begin(), v[i].second.end());
	sort(v.begin(), v.end());
	n--;
	vector<ll>w = v[n/ 2].second;
	reverse(w.begin(), w.end());
	if (n & 1)
		w[0] ^= 1;
	for (ll k = v[n >>(1)].first; k >= 0; --k)
		cout << ((w[k]*2)>>1 & 1);
	return 0;
}