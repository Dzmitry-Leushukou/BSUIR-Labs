using _353504_Levshukov_Lab5.Interfaces;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;

namespace _353504_Levshukov_Lab5.Collections
{

    public class Node<T>
    {
        public Node(T item)
        {
            Item = item;
        }

        public T Item { get; set; }
        public Node<T>? Next { get; set; }
    }
    internal class MyCustomCollection<T> : ICustomCollection<T>
    {
        private Node<T>? _begin, _end, _cur;
        public T this[int index]
        {
            get
            {
                Node<T>? cur = _begin;

                while (index>0)
                {
                    cur = cur!.Next;
                    index--;
                }

                if(cur!=null)
                return cur.Item;
                
                throw new IndexOutOfRangeException();
            }

            set 
            {
                if(index>=Count) throw new IndexOutOfRangeException();

                Node<T>? cur = _begin;

                while(index>0)
                {
                    cur = cur!.Next;
                    index--;
                }

                if (cur != null) cur.Item = value;
                else throw new IndexOutOfRangeException();
            }
        }
        public void Reset()
        {
            _cur = _begin;
        }
        public void Next()
        {
            _cur = _cur?.Next;
        }
        public T? Current()
        {
            if(_cur!=null)
                return _cur.Item;
            return default;
        }
        public int Count 
        { 
            get; private set;
        }
        public void Add(T item)
        {
            Node<T> new_node = new Node<T>(item);
            Count++;
            if (_begin==null)
            {
                _begin =_end=_cur= new_node;
                return;
            }

            _end!.Next = new_node;
            _end = _end!.Next;
        }
        public void Remove(T item)
        {
            Node<T>? cur = _begin;
            if (cur == null) return;
            if (cur.Item.Equals(item))
            {
                _begin = cur.Next;
                if (cur.Next == null)
                    _end = _cur = null;
                cur = null;
                Count--;
                return;
            }    

            while (cur.Next!=null&&!cur.Next.Item.Equals(item))
            {
                cur = cur!.Next;
            }

            if(cur.Next==null)return;
            if(cur.Next.Item.Equals(item))
            {
                if (cur.Next.Next != null)
                {
                    cur.Next = cur.Next.Next;
                }
                else
                {
                    _end = cur;
                    cur.Next = null;
                }
                Count--;
            }
        }
        public T RemoveCurrent()
        {
            T it = _cur!.Item;
            Remove(it);
            return it;
        }
    }
}
