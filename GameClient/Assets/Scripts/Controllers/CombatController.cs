using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CombatController : MonoBehaviour
{
    MeeleFighter _meeleFighter;

    private void Awake()
    {
        _meeleFighter = GetComponent<MeeleFighter>();
    }

    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        //  Left Mouse Button or Left Ctrl
        if(Input.GetButtonDown("Attack"))
        {
            _meeleFighter.TryToAttack();
        }
    }
}
