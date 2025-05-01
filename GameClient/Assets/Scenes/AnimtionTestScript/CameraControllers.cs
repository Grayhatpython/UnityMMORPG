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

        // 3. ��ǥ ȸ�� ���ʹϾ� ��� (Pitch�� Yaw �и� �� ����)
        // �¿� ȸ�� (Yaw): Y���� �߽����� _cameraYRotation ��ŭ ȸ��
        Quaternion yawRotation = Quaternion.Euler(0, _cameraYRotation, 0);

        // ���� ȸ�� (Pitch): X���� �߽����� _cameraXRotation ��ŭ ȸ��
        Quaternion pitchRotation = Quaternion.Euler(_cameraXRotation, 0, 0);

        // Yaw ȸ���� ���� �����ϰ�, �� ���¿��� Pitch ȸ���� �����Ͽ� ���� ��ǥ ȸ�� ���
        // (FPS�� 3��Ī ī�޶󿡼� ���� ���Ǵ� ȸ�� ����)
        Quaternion targetRotation = yawRotation * pitchRotation; // Yaw * Pitch ������ ����

        // 4. ���� ���� ȸ������ ���� targetRotation���� �ε巴�� ����
        Quaternion smoothedRotation = Quaternion.Slerp(_cameraCenter.transform.localRotation, targetRotation, _cameraSetting.rotationSpeed * Time.deltaTime);

        // 5. ������ ȸ�� ���� _cameraCenter�� ���� ȸ���� ����
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
