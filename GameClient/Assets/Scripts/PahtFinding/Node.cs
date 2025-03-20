using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[Serializable]
public class Node : IComparable<Node>
{   
    public bool     _isWalkable;
    public Vector3  _position;
    public int      _gridX;
    public int      _gridY;

    public int      _gCost;
    public int      _hCost;
    public Node     _parent;

    public Node(bool walkable, Vector3 position, int gridX, int gridY)
    {
        this._isWalkable = walkable;
        this._position = position;
        this._gridX = gridX;
        this._gridY = gridY;
    }

    public int CompareTo(Node otherNode)
    {
        if (this.fCost < otherNode.fCost) return -1;
        else if (this.fCost > otherNode.fCost) return 1;
        else return 0;
    }

    public int fCost
    {
        get
        {
            return _gCost + _hCost;
        }
    }
}
