using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

public class LoginScene : BaseScene
{
    public override void Clear()
    {
        Debug.Log("Login Scene Clear!");
    }

    protected override void Initialize()
    {
        base.Initialize();

        SceneType = Define.Scene.Login;

        //List<GameObject> list = new List<GameObject>();

        //for (int i = 0; i < 5; i++)
        //    list.Add(Managers.Resource.Instantiate("UnityChan"));

        //foreach (GameObject go in list)
        //    Managers.Resource.Destroy(go);
    }

    private void Update()
    {
        if(Input.GetKeyDown(KeyCode.Q))
        {
            Managers.Scene.LoadScene(Define.Scene.Game);
        }
    }
}
