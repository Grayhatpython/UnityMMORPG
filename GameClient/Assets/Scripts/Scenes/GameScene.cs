using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class GameScene : BaseScene
{

    public override void Clear()
    {

    }

    protected override void Initialize()
    {
        
        base.Initialize();

        SceneType = Define.Scene.Game;

        Dictionary<int, Data.Stat> dict = Managers.Data.StatDict;

        //Managers.Network.Start();

        //  Screen Size ¼³Á¤
        //Screen.SetResolution(854, 480, false);

        //for (int i = 0; i < 5; i++)
        //    Managers.Resource.Instantiate("UnityChan");
 
    }
}
