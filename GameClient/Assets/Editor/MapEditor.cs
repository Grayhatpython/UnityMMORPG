using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;

#if UNITY_EDITOR
using UnityEditor;
#endif

public class MapEditor 
{
#if UNITY_EDITOR

    [MenuItem("Tools/GenerateMap")]
    private static void GenerateMap()
    {
        GenerateMap("Assets/Resources/Map/map.txt");
        GenerateMap("../Common/Data/Map/map.txt");
    }

    private static void GenerateMap(string path)
    {
        /*
        GameObject go = GameObject.Find("World");

        if (go == null)
            return;

        WorldController world = go.GetComponent<WorldController>();

        if (world == null)
            return;

        world.CreateGrid();

        using (var writer = File.CreateText(path))
        {
            writer.WriteLine(world._nodeSize);
            writer.WriteLine(world.GridSizeX);
            writer.WriteLine(world.GridSizeY);
            writer.WriteLine(world.Grid[0, 0]._position.x);
            writer.WriteLine(world.Grid[0, world.GridSizeX - 1]._position.x);
            writer.WriteLine(world.Grid[0, 0]._position.z);
            writer.WriteLine(world.Grid[world.GridSizeY - 1, 0]._position.z);

            for (int y = world.GridSizeY - 1; y >= 0; y--)
            {
                for (int x = 0; x < world.GridSizeX; x++)
                {
                    bool isWalkable = world.Grid[y, x]._isWalkable;
                    if (isWalkable)
                        writer.Write("0");
                    else
                        writer.Write("1");
                }
                writer.WriteLine();
            }
        }
        */
    }

#endif
}
