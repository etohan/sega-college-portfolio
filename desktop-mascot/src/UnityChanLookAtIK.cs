using UnityEngine;

public class UnityChanLookAtIK : MonoBehaviour
{
    [Header("References")]
    [SerializeField] private Animator animator;
    [SerializeField] private Camera targetCamera;

    [Header("Look Target")]
    [SerializeField] private float lookDistance = 4.0f;
    [SerializeField] private float horizontalRange = 1.4f;
    [SerializeField] private float verticalRange = 0.8f;
    [SerializeField] private float smooth = 8.0f;

    [Header("IK Weight")]
    [Range(0f, 1f)]
    [SerializeField] private float lookWeight = 0.75f;

    [Range(0f, 1f)]
    [SerializeField] private float bodyWeight = 0.02f;

    [Range(0f, 1f)]
    [SerializeField] private float headWeight = 0.55f;

    [Range(0f, 1f)]
    [SerializeField] private float eyesWeight = 0.45f;

    [Range(0f, 1f)]
    [SerializeField] private float clampWeight = 0.65f;

    private Transform head;
    private Vector3 currentLookPosition;

    void Start()
    {
        if (animator == null)
        {
            animator = GetComponent<Animator>();
        }

        if (targetCamera == null)
        {
            targetCamera = Camera.main;
        }

        if (animator != null)
        {
            head = animator.GetBoneTransform(HumanBodyBones.Head);
        }

        currentLookPosition = CalculateLookPosition();
    }

    void Update()
    {
        Vector3 target = CalculateLookPosition();

        currentLookPosition = Vector3.Lerp(
            currentLookPosition,
            target,
            Time.deltaTime * smooth
        );
    }

    void OnAnimatorIK(int layerIndex)
    {
        if (animator == null) return;

        animator.SetLookAtWeight(
            lookWeight,
            bodyWeight,
            headWeight,
            eyesWeight,
            clampWeight
        );

        animator.SetLookAtPosition(currentLookPosition);
    }

    Vector3 CalculateLookPosition()
    {
        if (targetCamera == null)
        {
            return transform.position + transform.forward * lookDistance;
        }

        Vector3 basePosition;

        if (head != null)
        {
            basePosition = head.position;
        }
        else
        {
            basePosition = transform.position + Vector3.up * 1.5f;
        }

        Vector3 mouse = Input.mousePosition;

        float mouseX = (mouse.x / Screen.width - 0.5f) * 2f;
        float mouseY = (mouse.y / Screen.height - 0.5f) * 2f;

        mouseX = Mathf.Clamp(mouseX, -1f, 1f);
        mouseY = Mathf.Clamp(mouseY, -1f, 1f);

        Vector3 target =
            basePosition
            + targetCamera.transform.forward * lookDistance
            + targetCamera.transform.right * mouseX * horizontalRange
            + targetCamera.transform.up * mouseY * verticalRange;

        return target;
    }
}