using Google.Protobuf.WellKnownTypes;
using Protocol;
using Unity.VisualScripting;
using UnityEngine;
using UnityEngine.EventSystems;


public class PlayerController : MonoBehaviour
{
    public uint Id { get; set; }

    protected Animator      _animator;
    protected PlayerStat    _stat; 

    PositionInfo        _positionInfo = new PositionInfo();

    public PositionInfo PosInfo
    {
        get { return _positionInfo; }
        set 
        {
            if (_positionInfo.Equals(value))
                return;

            transform.position = new Vector3(value.X, value.Y, value.Z);
            transform.rotation = Quaternion.Euler(new Vector3(0.0f, _positionInfo.Yaw, 0.0f));
        }
    }
    PositionInfo _destPosInfo = new PositionInfo();
    public PositionInfo DestPosInfo
    {
        get { return _destPosInfo; }
        set
        {
            if (_destPosInfo.Equals(value))
                return;

            _destPosInfo = value;
        }
    }
    MoveState _state = MoveState.Idle;
    public MoveState State
    {
        get { return _state; }
        set
        {
            if (_state.Equals(value))
                return;

            _state = value;
        }
    }

    protected virtual void Initialize()
    {
        _animator = GetComponent<Animator>();   
        _stat = GetComponent<PlayerStat>();
    }

    #region Update
    protected virtual void UpdateAnimation()
    {
        if (_animator == null)
            return;

        float moveSpeedValue;
        if (State == MoveState.Idle)
            moveSpeedValue = 0;
        else
            moveSpeedValue = 1;

        _animator.SetFloat("moveSpeed", moveSpeedValue, .15f, Time.deltaTime);
    }

    protected virtual void UpdateController()
    {
        UpdateAnimation();

        _positionInfo.X = transform.position.x;
        _positionInfo.Y = transform.position.y;
        _positionInfo.Z = transform.position.z;
        _positionInfo.Yaw = transform.rotation.eulerAngles.y;

        switch (State)
        {
            case MoveState.Idle:
                UpdateIdle();
                break;
            case MoveState.Run:
                UpdateMove();
                break;
            case MoveState.Skill:
                UpdateSkill();
                break;
        }
    }

    protected virtual void UpdateIdle()
    {

    }

    protected virtual void UpdateMove()
    {
        Quaternion destRotation = Quaternion.Euler(0.0f, _destPosInfo.Yaw, 0.0f);
        transform.rotation = Quaternion.Slerp(transform.rotation, destRotation, 10 * Time.deltaTime);
        //transform.rotation = destRotation;

        Vector3 forward = destRotation * Vector3.forward;
        Vector3 targetPosition = transform.position + forward;
        targetPosition.y = 0;
        transform.position = Vector3.MoveTowards(transform.position, targetPosition, _stat.MoveSpeed * Time.deltaTime);
   
        //  목적지까지 도착하지 못했다면..?
    }

    protected virtual void UpdateSkill()
    {

    }

    #endregion

    void Start()
    {
        Initialize();
    }

    void Update()
    {
        UpdateController();
    }
}
