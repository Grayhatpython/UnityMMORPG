using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;

public class CameraControllers : MonoBehaviour
{
    [System.Serializable]
    public class CameraSetting
    {
        [Header("Camera Move Setting")]
        public float zoomSpeed = 5.0f;
        public float moveSpeed = 5.0f;
        public float rotationSpeed = 5.0f;
        public float originalFieldOfView = 70.0f;
        public float zoomFieldOfView = 50.0f;
        public float mouseXSensitivity = 5.0f;
        public float mouseYSensitivity = 5.0f;
        public float maxAngle = 90.0f;
        public float minAngle = -30.0f;

        [Header("Caemra Collision")]
        public Transform cameraPosition;
        public LayerMask cameraCollisionLayers;
    }

    [SerializeField]
    public CameraSetting _cameraSetting;

    [System.Serializable]
    public class CameraInputSetting
    {
        public string mouseXAxis = "Mouse X";
        public string mouseYAxis = "Mouse Y";
        public string aimingInput = "Fire2";
    }
    [SerializeField]
    public CameraInputSetting _cameraInput;

    Transform _cameraCenter;
    Transform _target;
    Camera _mainCamera;

    float _cameraXRotation = 0.0f;
    float _cameraYRotation = 0.0f;

    Vector3 _initilzeCameraPosition;
    RaycastHit _hit;

    // Start is called before the first frame update
    void Start()
    {
        _mainCamera = Camera.main;
        _cameraCenter = transform.GetChild(0);
        _initilzeCameraPosition = _mainCamera.transform.localPosition;
        FindTarget();
    }

    // Update is called once per frame
    void Update()
    {
        if (_target == null)
            return;


        if (Application.isPlaying == false)
            return;

        RotateCamera();
        ZoomCamera();
        HandleCameraCollision();
    }

    private void LateUpdate()
    {
        if (_target)
        {
            FollowTarget();
        }
        else
        {
            FindTarget();
        }

        //FollowTarget();
    }

    void FindTarget()
    {
        _target = GameObject.FindGameObjectWithTag("Player").transform;
    }

    void FollowTarget()
    {
        Vector3 moveVector = Vector3.Lerp(transform.position, _target.transform.position, _cameraSetting.moveSpeed * Time.deltaTime);

        transform.position = moveVector;
    }

    void RotateCamera()
    {
        _cameraXRotation += -Input.GetAxis(_cameraInput.mouseYAxis) * _cameraSetting.mouseYSensitivity;
        _cameraYRotation += Input.GetAxis(_cameraInput.mouseXAxis) * _cameraSetting.mouseXSensitivity;

        _cameraXRotation = Mathf.Clamp(_cameraXRotation, _cameraSetting.minAngle, _cameraSetting.maxAngle);
        _cameraYRotation = Mathf.Repeat(_cameraYRotation, 360);

        // 3. 목표 회전 쿼터니언 계산 (Pitch와 Yaw 분리 후 결합)
        // 좌우 회전 (Yaw): Y축을 중심으로 _cameraYRotation 만큼 회전
        Quaternion yawRotation = Quaternion.Euler(0, _cameraYRotation, 0);

        // 상하 회전 (Pitch): X축을 중심으로 _cameraXRotation 만큼 회전
        Quaternion pitchRotation = Quaternion.Euler(_cameraXRotation, 0, 0);

        // Yaw 회전을 먼저 적용하고, 그 상태에서 Pitch 회전을 적용하여 최종 목표 회전 계산
        // (FPS나 3인칭 카메라에서 흔히 사용되는 회전 순서)
        Quaternion targetRotation = yawRotation * pitchRotation; // Yaw * Pitch 순서로 곱함

        // 4. 현재 로컬 회전에서 계산된 targetRotation까지 부드럽게 보간
        Quaternion smoothedRotation = Quaternion.Slerp(_cameraCenter.transform.localRotation, targetRotation, _cameraSetting.rotationSpeed * Time.deltaTime);

        // 5. 보간된 회전 값을 _cameraCenter의 로컬 회전에 적용
        _cameraCenter.transform.localRotation = smoothedRotation;

    }

    void ZoomCamera()
    {
        if(Input.GetButton(_cameraInput.aimingInput))
        {
            _mainCamera.fieldOfView = Mathf.Lerp(_mainCamera.fieldOfView, _cameraSetting.zoomFieldOfView, _cameraSetting.zoomSpeed * Time.deltaTime);
        }
        else
        {
            _mainCamera.fieldOfView = Mathf.Lerp(_mainCamera.fieldOfView, _cameraSetting.originalFieldOfView, _cameraSetting.zoomSpeed * Time.deltaTime);
        }
    }

    void HandleCameraCollision()
    {
        if (Application.isPlaying == false)
            return;

        if(Physics.Linecast(_target.transform.position + _target.transform.up, _cameraSetting.cameraPosition.position, out _hit, _cameraSetting.cameraCollisionLayers))
        {
            Vector3 cameraPosition = new Vector3(_hit.point.x + _hit.normal.x * .2f, _hit.point.y + _hit.normal.y * .8f, _hit.point.z + _hit.normal.z * .2f);
            _mainCamera.transform.position = Vector3.Lerp(_mainCamera.transform.position, cameraPosition, Time.deltaTime * _cameraSetting.moveSpeed);
        }
        else
        {
            _mainCamera.transform.localPosition = Vector3.Lerp(_mainCamera.transform.localPosition, _initilzeCameraPosition, Time.deltaTime * _cameraSetting.moveSpeed);
        }

        Debug.DrawLine(_target.transform.position + _target.transform.up, _cameraSetting.cameraPosition.position, Color.green);
    }
}
