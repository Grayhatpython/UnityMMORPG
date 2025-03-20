using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PathFinding : MonoBehaviour
{
    WorldController _world;

    public Transform _start;
    public Transform _destination;
    private Vector3 _cacheStart;
    private Vector3 _cacheDest;

    private void Awake()
    {
        _world = GetComponent<WorldController>();
    }

    void Update()
    {
        if (_start.position != _cacheStart || _destination.position != _cacheDest)
        {
            FindPath(_start.position, _destination.position);

            _cacheStart = _start.position;
            _cacheDest = _destination.position;
        }
    }

    public void FindPath(Vector3 startPos, Vector3 targetPos)
    {
        Node startNode = _world.GetNodeFromPosition(startPos);
        Node targetNode = _world.GetNodeFromPosition(targetPos);

        List<Node> openSet = new List<Node>();
        HashSet<Node> closedSet = new HashSet<Node>();

        openSet.Add(startNode);
        while (openSet.Count > 0)
        {
            #region ���� ���� ���� ���� ��带 �����Ѵ�.
            Node currentNode = openSet[0];
            for (int i = 1; i < openSet.Count; i++)
            {
                if (openSet[i].fCost < currentNode.fCost || (openSet[i].fCost == currentNode.fCost && openSet[i]._hCost < currentNode._hCost))
                {
                    currentNode = openSet[i];
                }
            }
            #endregion

            #region ���� ���� ���� ���� ��尡 �������� Ž���� �����Ѵ�.
            if (currentNode == targetNode)
            {
                RetracePath(startNode, targetNode);
                return;
            }
            #endregion

            #region ���� ��带 ���� �¿��� ���� Ŭ����� ������ �̵��Ѵ�.
            openSet.Remove(currentNode);
            closedSet.Add(currentNode);
            #endregion

            #region �̿���带 �����ͼ� ���� ����� �� ���� �¿� �߰��Ѵ�.
            foreach (Node n in _world.GetNeighbours(currentNode))
            {
                if (!n._isWalkable || closedSet.Contains(n))
                {
                    continue;
                }

                int g = currentNode._gCost + GetDistance(currentNode, n);
                int h = GetDistance(n, targetNode);
                int f = g + h;

                // ���� �¿� �̹� �ߺ� ��尡 �ִ� ��� ���� ���� ������ �����Ѵ�.
                if (!openSet.Contains(n))
                {
                    n._gCost = g;
                    n._hCost = h;
                    n._parent = currentNode;
                    openSet.Add(n);
                }
                else
                {
                    if (n.fCost > f)
                    {
                        n._gCost = g;
                        n._parent = currentNode;
                    }
                }
            }
            #endregion
        }
    }

    void RetracePath(Node startNode, Node endNode)
    {
        List<Node> path = new List<Node>();
        Node currentNode = endNode;

        while (currentNode != startNode)
        {
            path.Add(currentNode);
            currentNode = currentNode._parent;
        }

        path.Reverse();
        _world.Path = path;
    }

    int GetDistance(Node nodeA, Node nodeB)
    {
        int dstX = Mathf.Abs(nodeA._gridX - nodeB._gridX);
        int dstY = Mathf.Abs(nodeA._gridY - nodeB._gridY);

        if (dstX > dstY)
        {
            return 14 * dstY + 10 * (dstX - dstY);
        }

        return 14 * dstX + 10 * (dstY - dstX);
    }

}
