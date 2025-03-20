using System;
using System.Collections;
using System.Collections.Generic;
using System.Threading;
using UnityEngine;
using UnityEngine.UI;

public class PriorityQueue<T> where T : IComparable<T>
{
    public List<T> data;

    public PriorityQueue()
    {
        this.data = new List<T>();
    }

    public void Enqueue(T item)
    {
        data.Add(item);
        BubbleUp();
    }

    public void Reposition(T item)
    {
        int index = data.IndexOf(item);
        while (index > 0)
        {
            int pi = (index - 1) / 2;
            if (data[index].CompareTo(data[pi]) >= 0)
            {
                break;
            }
            Swap(index, pi);
            index = pi;
        }
    }

    private void BubbleUp()
    {
        int ci = Count - 1;
        while (ci > 0)
        {
            int pi = (ci - 1) / 2;
            if (data[ci].CompareTo(data[pi]) >= 0)
            {
                break;
            }
            Swap(ci, pi);
            ci = pi;
        }
    }

    private void Swap(int index1, int index2)
    {
        var tmp = data[index1];
        data[index1] = data[index2];
        data[index2] = tmp;
    }

    public T Dequeue()
    {
        T head = data[0];

        MoveLastItemToTheTop();
        SinkDown();

        return head;
    }

    private void MoveLastItemToTheTop()
    {
        int li = Count - 1;
        data[0] = data[li];
        data.RemoveAt(li);
    }

    private void SinkDown()
    {
        var li = Count - 1;
        var pi = 0;

        while (true)
        {
            var ci1 = pi * 2 + 1;
            if (ci1 > li)
            {
                break;
            }
            var ci2 = ci1 + 1;
            if (ci2 <= li && data[ci2].CompareTo(data[ci1]) < 0)
            {
                ci1 = ci2;
            }
            if (data[pi].CompareTo(data[ci1]) < 0)
            {
                break;
            }
            Swap(pi, ci1);
            pi = ci1;
        }
    }

    public bool Contains(T nodeToCheck)
    {
        return data.Contains(nodeToCheck);
    }

    public int Count
    {
        get
        {
            return data.Count;
        }
    }

    public T Head
    {
        get
        {
            return data[0];
        }
    }
}
