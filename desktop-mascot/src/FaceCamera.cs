using UnityEngine;

public class FaceCamera : MonoBehaviour
{
    private Camera mainCamera; // メインカメラ

    void Start()
    {
        // メインカメラを取得
        mainCamera = Camera.main;
    }

    void Update()
    {
        // モデルをカメラの方向に向ける
        Vector3 targetPosition = new Vector3(mainCamera.transform.position.x, transform.position.y, mainCamera.transform.position.z);
        transform.LookAt(targetPosition);
    }
}
