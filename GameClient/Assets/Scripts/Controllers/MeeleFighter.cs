using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public enum AttackState
{ 
    Idle,
    Windup,
    Impact,
    Cooldown,
}

public class MeeleFighter : MonoBehaviour
{
    [SerializeField] 
    List<AttackData> _attacks;

    [SerializeField]
    GameObject _sword;

    BoxCollider _swordCollider;

    Animator _animator;
    public bool InAction { get; private set; } = false;

    AttackState _attackState;

    private void Awake()
    {
        _animator = GetComponent<Animator>();
    }

    private void Start()
    {
        if(_sword != null)
        {
            _swordCollider = _sword.GetComponent<BoxCollider>();
            _swordCollider.enabled = false;
        }
    }

    bool _doCombo;
    int _comboCount = 0;

    public void TryToAttack()
    {
        if(InAction == false)
        {
            StartCoroutine(CoAttack());
        }
        else if(_attackState == AttackState.Impact || _attackState == AttackState.Cooldown)
        {
            _doCombo = true;
        }
    }

    IEnumerator CoAttack()
    {
        InAction = true;
        _attackState = AttackState.Windup;

        _animator.CrossFade(_attacks[_comboCount].AnimName, 0.2f);
        //  코루틴 실행을 현재 프레임의 끝에서 일시 중지하고, 다음 프레임이 시작될 때 다시 이어서 실행
        yield return null;

        //  Combat Animation
        int overrideLayerIndex = 1;

        //  Next Animation State 
        AnimatorStateInfo animState = _animator.GetNextAnimatorStateInfo(overrideLayerIndex);

        float timer = 0.0f;
        while(timer <= animState.length)
        {
            timer += Time.deltaTime;
            float normalizedTime = timer / animState.length;

            if(_attackState == AttackState.Windup)
            {
                if (normalizedTime > _attacks[_comboCount].ImpactStartTime)
                {
                    _attackState = AttackState.Impact;
                    // Enable Colider
                    _swordCollider.enabled = true;
                }

            }else if(_attackState == AttackState.Impact)
            {
                if (normalizedTime > _attacks[_comboCount].ImpactEndTime)
                {
                    _attackState = AttackState.Cooldown;
                    //  Disable Colider
                    _swordCollider.enabled = false;
                }
            }else if(_attackState == AttackState.Cooldown)
            {
                //  TODO
                if(_doCombo)
                {
                    _doCombo = false;
                    _comboCount = (_comboCount + 1) % _attacks.Count;

                    StartCoroutine(CoAttack());
                    yield break;
                }
            }

            yield return null;
        }

        //  Slash Animation length
        //yield return new WaitForSeconds(animState.length);

        _attackState = AttackState.Idle;
        _comboCount = 0;
        InAction = false;
    }


    IEnumerator CoAttacked()
    {
        InAction = true;
        _animator.CrossFade("SwordImpact", 0.2f);
        //  코루틴 실행을 현재 프레임의 끝에서 일시 중지하고, 다음 프레임이 시작될 때 다시 이어서 실행
        yield return null;

        //  Combat Animation
        int overrideLayerIndex = 1;

        //  Next Animation State 
        AnimatorStateInfo animState = _animator.GetNextAnimatorStateInfo(overrideLayerIndex);

        //  Slash Animation length
        yield return new WaitForSeconds(animState.length);

        InAction = false;
    }

    //  쳐맞을때
    private void OnTriggerEnter(Collider other)
    {
        if(other.tag == "Hitbox" && InAction == false)
        {
            StartCoroutine(CoAttacked());

            Debug.Log(gameObject.name);
            Debug.Log(other.name);
            Debug.Log("Character was hit");
        }
    }
}
