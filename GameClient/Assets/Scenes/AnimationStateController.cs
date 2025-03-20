using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class AnimationStateController : MonoBehaviour
{
    //Animator _animator;
    //int _isWalkingHash;
    //int _isRunningHash;

    //// Start is called before the first frame update
    //void Start()
    //{
    //    _animator = GetComponent<Animator>();
    //    _isWalkingHash = Animator.StringToHash("isWalking");
    //    _isRunningHash = Animator.StringToHash("isRunning");
    //}

    //// Update is called once per frame
    //void Update()
    //{
    //    bool isRunning = _animator.GetBool(_isRunningHash);
    //    bool isWalking = _animator.GetBool(_isWalkingHash);
    //    bool forwardPressed = Input.GetKey("w");
    //    bool runPressed = Input.GetKey("left shift");

    //    if(isWalking == false && forwardPressed)
    //    {
    //        _animator.SetBool(_isWalkingHash, true);
    //    }
    //    if(isWalking && forwardPressed == false)
    //    {
    //        _animator.SetBool(_isWalkingHash, false);
    //    }

    //    if(isRunning == false && (forwardPressed && runPressed))
    //    {
    //        _animator.SetBool(_isRunningHash, true);
    //    }

    //    if(isRunning && (forwardPressed == false || runPressed == false))
    //    {
    //        _animator.SetBool(_isRunningHash, false);
    //    }
    //}
    Animator _animator;
    float velocity = 0.0f;
    public float acceleration = 0.1f;
    public float deceleration = 0.5f;
    int velocityHash;
    // Start is called before the first frame update
    void Start()
    {
        _animator = GetComponent<Animator>();

        velocityHash = Animator.StringToHash("Velocity");
    }

    // Update is called once per frame
    void Update()
    {
        bool forwardPressed = Input.GetKey("w");
        bool runPressed = Input.GetKey("left shift");

        if(forwardPressed && velocity < 1.0f)
        {
            velocity += Time.deltaTime * acceleration;
        }

        if(forwardPressed == false && velocity > 0.0f)
        {
            velocity -= Time.deltaTime * deceleration;
        }

        if(forwardPressed == false && velocity < 0.0f)
        {
            velocity = 0.0f;
        }
        _animator.SetFloat(velocityHash, velocity);
    }
}
