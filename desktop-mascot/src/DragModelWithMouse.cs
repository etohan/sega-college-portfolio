using UnityEngine;

public class DragModelWithMouse : MonoBehaviour
{
    private Camera mainCamera; // メインカメラ
    private bool isDragging = false; // ドラッグ中かどうか
    private Vector3 offset; // 初期位置とマウス位置との差

    void Start()
    {
        // メインカメラを取得
        mainCamera = Camera.main;
    }

    void Update()
    {
        // 左クリックでドラッグ開始
        if (Input.GetMouseButtonDown(0)) // 0は左クリック
        {
            // モデルにカーソルがある場合
            Ray ray = mainCamera.ScreenPointToRay(Input.mousePosition);
            RaycastHit hit;
            if (Physics.Raycast(ray, out hit))
            {
                if (hit.collider.gameObject == gameObject)
                {
                    isDragging = true;

                    // マウスの位置とオブジェクトのワールド座標を基準にオフセットを計算
                    offset = transform.position - hit.point;
                }
            }
        }

        // ドラッグ中の場合
        if (isDragging)
        {
            // マウスのワールド座標を計算してオブジェクトを移動
            Vector3 mousePosition = Input.mousePosition;
            mousePosition.z = mainCamera.WorldToScreenPoint(transform.position).z; // オブジェクトのZ座標を保持
            Vector3 worldPosition = mainCamera.ScreenToWorldPoint(mousePosition) + offset;

            // Y軸とX軸の位置を更新（Z軸は変更しない）
            transform.position = new Vector3(worldPosition.x, worldPosition.y, transform.position.z);
        }

        // 左クリックを離したらドラッグ終了
        if (Input.GetMouseButtonUp(0)) // 0は左クリック
        {
            isDragging = false;
        }
    }
}
