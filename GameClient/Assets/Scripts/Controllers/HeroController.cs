using Google.Protobuf.Protocol;
using Protocol;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.InputSystem;

public class HeroController : PlayerController
{
    PlayerInputController _inputController;
    //CharacterController _characterController;

    Vector2 _moveInput = Vector2.zero;
    Vector2 _prevMoveInput = Vector2.zero;
    Vector3 _moveDirection = Vector3.zero ;
    float   _desiredYaw = 0.0f;

    const float MOVE_PACKET_SEND_TICK = 0.2f;
    float _calcMovePacketSendTime = MOVE_PACKET_SEND_TICK;

    //  TEMP
    bool _skillInput = false;
    float _skillRange = 1f;

    private void Awake()
    {
        InputInitialize();
    }

    private void InputInitialize()
    {
        _inputController = new PlayerInputController();

        _inputController.Character.Movement.performed += context => _moveInput = context.ReadValue<Vector2>();
        _inputController.Character.Movement.canceled += context => _moveInput = Vector2.zero;

        _inputController.Character.Attack.performed += context => _skillInput = context.ReadValueAsButton();
        _inputController.Character.Attack.canceled += context => _skillInput = false;
    }

    protected override void Initialize()
    {
        base.Initialize();

        //_characterController = GetComponent<CharacterController>();
    }

    #region Update
    protected override void UpdateController()
    {
        UpdateInput();
        base.UpdateController();
        UpdateMovePacket();

        //  Draw Ray Debug
        Vector3 origin = transform.position + Vector3.up;
        Vector3 direction = transform.forward;
        Debug.DrawRay(origin, direction * _skillRange, Color.red);
    }

    protected override void UpdateIdle()
    {

    }

    protected override void UpdateMove()
    {
        Vector3 targetPosition = transform.position + _moveDirection;
        targetPosition.y = 0;
        transform.position = Vector3.MoveTowards(transform.position, targetPosition, _stat.MoveSpeed * Time.deltaTime);

        Quaternion rotate = Quaternion.LookRotation(_moveDirection);
        _desiredYaw = rotate.eulerAngles.y;

        //Quaternion destRotation = Quaternion.Euler(0.0f, _desiredYaw, 0.0f);
        transform.rotation = Quaternion.Slerp(transform.rotation, rotate, 10 * Time.deltaTime);

        //_characterController.Move(_moveDirection * _moveSpeed * Time.deltaTime);
    }

    Coroutine _coSkillCooldown = null;

    protected override void UpdateSkill()
    {
        
    }

    IEnumerator CoSkillAttack(float waitTime)
    {
        //  TEST
        RaycastHit hit;

        Vector3 origin = transform.position + Vector3.up;
        Vector3 direction = transform.forward;
        if (Physics.Raycast(origin, direction, out hit, _skillRange, 1 << (int)Define.Layer.Monster))
        {
            Debug.Log("fdsf");
        }

        yield return new WaitForSeconds(waitTime);
        _coSkillCooldown = null;

        State = MoveState.Idle;
    }

    private void UpdateInput()
    {
        if (State == MoveState.Skill)
            return;

        //  TEMP
        if (_coSkillCooldown == null && _skillInput)
        {
            //  음.. 서버 허락받고 쓸지 바로 쓸지 고민
            State = MoveState.Skill;

            _coSkillCooldown = StartCoroutine("CoSkillAttack", 0.5f); 
            return;
        }

        Vector3 forward = Camera.main.transform.forward;    
        Vector3 right = Camera.main.transform.right;
        forward.y = 0;
        right.y = 0;

        forward.Normalize();
        right.Normalize();

        _moveDirection = forward * _moveInput.y + right * _moveInput.x;  

        if (_moveDirection.magnitude > 0)
            State = MoveState.Run;
        else
            State = MoveState.Idle;
    }

    private void UpdateMovePacket()
    {
        bool focreMovePacketSend = false;

        if (_prevMoveInput != _moveInput)
        {
            focreMovePacketSend = true;
            _prevMoveInput = _moveInput;
        }

        _calcMovePacketSendTime -= Time.deltaTime;

        if (_calcMovePacketSendTime <= 0 || focreMovePacketSend)
        {
            _calcMovePacketSendTime = MOVE_PACKET_SEND_TICK;

            C_MOVE movePacket = new C_MOVE();
            movePacket.PositionInfo = PosInfo;
            movePacket.PositionInfo.ObjectId = Id;
            movePacket.PositionInfo.State = State;
            movePacket.PositionInfo.Yaw = _desiredYaw;
            Managers.Network.Send(movePacket);
        }
    }

    #endregion

    private void OnEnable()
    {
        _inputController.Enable();
    }

    private void OnDisable()
    {
        _inputController.Disable();
    }
}
