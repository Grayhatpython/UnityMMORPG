using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[RequireComponent(typeof(Animator))]
[RequireComponent(typeof(CharacterController))]
public class Movement : MonoBehaviour
{
    CharacterController _characterController;
    Animator _animator;

    [System.Serializable]
    public class AnimationString
    {
        public string horizontal = "horizontal";
        public string vertical = "vertical";
        public string sprint = "sprint";
        public string aim = "aim";
        public string fire = "fire";
        public string pullString = "pullString";
    }

    [SerializeField]
    public AnimationString _animationString;


    // Start is called before the first frame update
    void Start()
    {
        _characterController = GetComponent<CharacterController>();
        _animator = GetComponent<Animator>();
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    public void AnimateCharacter(float horizontal, float vertical)
    {
        _animator.SetFloat(_animationString.horizontal, horizontal);
        _animator.SetFloat(_animationString.vertical, vertical);
    }

    public void SprintCharacter(bool isSprint)
    {
        _animator.SetBool(_animationString.sprint, isSprint);
    }

    public void CharacterAim(bool isAim)
    {
        _animator.SetBool(_animationString.aim, isAim);
    }

    public void CharacterPullString(bool isPullString)
    {
        _animator.SetBool(_animationString.pullString, isPullString);
    }

    public void CharacterFireArrow()
    {
        _animator.SetTrigger(_animationString.fire);
    }
}
