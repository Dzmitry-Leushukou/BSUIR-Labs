using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LAB
{
    internal class Matrix
    {
        private int[, ] data=new int [2,2];

        public int this[int x, int y]
        {
            get { return data[x, y]; }
            set { data[x, y] = value; }
        }

        public string ToString()
        {
            return $"{data[0,0]} {data[0, 1]}\n{data[1, 0]} {data[1, 1]}";
        }
        public static Matrix operator +(Matrix a, Matrix b)
        {
            var result = new Matrix();
            for (int i = 0; i < 2; i++)
            {
                for (int j = 0; j < 2; j++)
                {
                    result[i, j] = a[i, j] + b[i, j];
                }
            }
            return result;
        }

        public static Matrix operator -(Matrix a, Matrix b)
        {
            var result = new Matrix();
            for (int i = 0; i < 2; i++)
            {
                for (int j = 0; j < 2; j++)
                {
                    result[i, j] = a[i, j] - b[i, j];
                }
            }
            return result;
        }

        public static Matrix operator *(Matrix a, Matrix b)
        {
            var result = new Matrix();
            for (int i = 0; i < 2; i++)
             {
                for (int j = 0; j < 2; j++)
                {
                    result[i, j] =0;
                    for(int k=0;k<2;k++)
                    {
                        result[i,j] += a[i, k] * b[k, i];
                    }
                }
            }
            return result;
        }

        public static Matrix operator *(Matrix a, int b)
        {
            var result = new Matrix();
            for (int i = 0; i < 2; i++)
            {
                for (int j = 0; j < 2; j++)
                {
                    result[i, j] = a[i,j] * b;
                }
            }
            return result;
        }

        public static Matrix operator /(Matrix a, int b)
        {
            var result = new Matrix();
            for (int i = 0; i < 2; i++)
            {
                for (int j = 0; j < 2; j++)
                {
                    result[i, j] = a[i, j] / b;
                }
            }
            return result;
        }

        public static Matrix operator ++(Matrix a)
        {
            for (int i = 0; i < 2; i++)
            {
                for (int j = 0; j < 2; j++)
                {
                    a[i, j]++;
                }
            }
            return a;
        }

        public static Matrix operator --(Matrix a)
        {
            for (int i = 0; i < 2; i++)
            {
                for (int j = 0; j < 2; j++)
                {
                    a[i, j]--;
                }
            }
            return a;
        }

        public static bool operator ==(Matrix a, Matrix b)
        {
            return a.Equals(b);
        }

        public static bool operator !=(Matrix a, Matrix b)
        {
            return !a.Equals(b);
        }

        public override bool Equals(object obj)
        {
            if (obj is Matrix m)
            {
                for (int i = 0; i < 2; i++)
                {
                    for (int j = 0; j < 2; j++)
                    {
                        if (data[i, j] != m[i, j])
                            return false;
                    }
                }
                return true;
            }
            return false;
        }


        public static explicit operator int(Matrix m)
        {
            return m[0, 0] * m[1, 1] - m[0, 1] * m[1, 0];
        }

        public static explicit operator Matrix(int determinant)
        {
            if (determinant == 0)
                return new Matrix(); 
            else
            {
                var m = new Matrix();
                m[0, 0] = m[1, 1] = determinant;
                m[1, 0] = m[0, 1] = 1;
                m[1, 0] = m[0, 1] = 0;
                return m;
            }
        }

        public bool IsZeroDeterminant()
        {
            return (int)this == 0;
        }

        public Matrix()
        {
            data[0, 0] = data[0, 1] = data[1, 0] = data[1, 1] = 0;
        }

    }
}
