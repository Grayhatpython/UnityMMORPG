using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerController : MonoBehaviour
{
    [SerializeField]
    float _moveSpeed = 5.0f;

    [SerializeField]
    float _rotationSpeed = 800.0f;

    CameraController _cameraController;

    Quaternion _moveRotation;

    Animator _animator;

    CharacterController _characterController;

    MeeleFighter _meeleFighter;
    private void Awake()
    {
        _cameraController = Camera.main.GetComponent<CameraController>();
        _characterController = GetComponent<CharacterController>();
        _animator = GetComponent<Animator>();
        _meeleFighter = GetComponent<MeeleFighter>();
    }

    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        if (_meeleFighter.InAction)
        {
            _animator.SetFloat("moveSpeed", 0.0f);
            return;
        }

        float horizontal = Input.GetAxis("Horizontal");
        float vertical = Input.GetAxis("Vertical");

        float moveInputValue = Mathf.Clamp01(Mathf.Abs(horizontal) + Mathf.Abs(vertical));

        Vector3 moveInput = new Vector3(horizontal, 0, vertical).normalized;
        Vector3 moveDir = _cameraController.PlanarRotation * moveInput;


        if (moveInputValue > 0)
        {
            _characterController.Move(moveDir * _moveSpeed * Time.deltaTime);
            //transform.position += moveDir * _moveSpeed * Time.deltaTime;
            _moveRotation = Quaternion.LookRotation(moveDir);
        }

        transform.rotation = Quaternion.RotateTowards(transform.rotation, _moveRotation, _rotationSpeed * Time.deltaTime);

        _animator.SetFloat("moveSpeed",moveInputValue, 0.2f, Time.deltaTime);
    }
}
