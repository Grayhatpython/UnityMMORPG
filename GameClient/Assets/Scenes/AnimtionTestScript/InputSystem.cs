using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[RequireComponent(typeof(Movement))]
public class InputSystem : MonoBehaviour
{
    Movement _movement;

    [System.Serializable]
    public class InputSetting
    {
        public string horizontalInput = "Horizontal";
        public string verticalInput = "Vertical";
        public string sprintInput = "Sprint";
        public string aimInput = "Fire2";
        public string fireInput = "Fire1";
    }

    [SerializeField]
    public InputSetting _input;

    [Header("Camera & Character Sync")]
    public float _lookDistance = 5.0f;
    public float _lookSpeed = 5.0f;

    [Header("Aiming Setting")]
    RaycastHit _hit;
    public LayerMask _aimLayers;
    Ray _ray;

    [Header("Spine Setting")]
    public Transform _spine;
    public Vector3 _spineOffset;

    [Header("Head Rotation Setting")]
    public float _lookAtPoint = 2.8f;

    Transform _cameraCenter;
    Transform _mainCamera;

    public Bow _bow;

    // Start is called before the first frame update
    void Start()
    {
        _movement = GetComponent<Movement>();
        _cameraCenter = Camera.main.transform.parent;
        _mainCamera = Camera.main.transform;
    }

    bool _isAim;
    public bool _testAim;

    // Update is called once per frame
    void Update()
    {
        if(Input.GetAxis(_input.verticalInput) != 0 || Input.GetAxis(_input.horizontalInput) != 0)
        {
            RotateToCameraView();
        }


        _isAim = Input.GetButton(_input.aimInput);

        if (_testAim)
            _isAim = true;

        _movement.AnimateCharacter(Input.GetAxis(_input.horizontalInput), Input.GetAxis(_input.verticalInput));
        _movement.SprintCharacter(Input.GetButton(_input.sprintInput));

        _movement.CharacterAim(_isAim);
        if (_isAim)
        {
            Debug.Log("fdfd");
            Aim();
            _movement.CharacterPullString(Input.GetButton(_input.fireInput));
        }
    }

    private void LateUpdate()
    {
        if (_isAim)
            RotateCharacterSpine();
    }

    void RotateToCameraView()
    {
        Vector3 cameraCenterPosition = _cameraCenter.position;
        Vector3 lookPoint = cameraCenterPosition + (_cameraCenter.forward * _lookDistance);
        Vector3 moveDir = lookPoint - transform.position;

        Quaternion lookRotation = Quaternion.LookRotation(moveDir);
        lookRotation.x = 0;
        lookRotation.z = 0;

        Quaternion rotation = Quaternion.Lerp(transform.rotation, lookRotation, Time.deltaTime * _lookSpeed);
        transform.rotation = rotation;
    }

    //  조준하고 대상에 레이캐스트를 보냅니다.
    void Aim()
    {
        Vector3 cameraPosition = _mainCamera.position;
        Vector3 dir = _mainCamera.forward;

        _ray = new Ray(cameraPosition, dir);
        if(Physics.Raycast(_ray, out _hit, 500f, _aimLayers))
        {
            Debug.Log(_hit.transform.name);
            Debug.DrawLine(_ray.origin, _hit.point, Color.red);
            _bow.ShowCrosshair(_hit.point);
        }
        else
        {
            _bow.RemoveCrosshair();
        }
    }

    void RotateCharacterSpine()
    {
        _spine.LookAt(_ray.GetPoint(50));
        _spine.Rotate(_spineOffset);
    }
}
