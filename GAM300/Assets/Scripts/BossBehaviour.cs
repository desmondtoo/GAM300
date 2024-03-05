﻿using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using BeanFactory;
using GlmSharp;

public class BossBehaviour : Script
{

    private enum BossState
    {
        Chase,
        BasicAttack,
        DashAttack,
        SlamAttack,
        UltimateAttack,
        UltimateCharge,
        ProjectileAttack,
    }

    //Start boss fight - DashAttacks towards you to close the distance
    //1st phase rotation:
    //  Chase 3s, if within distance, basic attack, else dash attack
    //  If after 3 basic attacks, jump back, dash attack

    //Transition:
    //  Health drops to 0 for the first time, enters sphere shield, doesnt take damage, does big aoe damage after charging up

    //2nd phase rotation
    //  Starts of with slam attack
    //  Then throws projectile in a 360* angle

    BossState state = 0;

    public float chaseSpeed = 10f;
    public float rotationSpeed = 10f;

    public float projectileSpeed = 10f;

    public Animator animator;

    public float basicAttackDistance = 3f;

    public float chaseDuration = 3f;

    public float dodgeDuration = 2f;

    public float ultimateSize = 66f;

    public float basicAttackDuration = 3f;

    public float dashAttackDuration = 3f;

    public float jumpAttackDuration = 3f;

    public float ultiChargeDuration = 10f;

    public float projectileAttackDuration = 4f;

    public float ultiExplodeDuration = 2f;

    public float slamAttackRadius = 10f;

    vec3 indicatorLocal = new vec3();

    public Transform ultiSphere;

    AnimationStateMachine animationManager;

    ThirdPersonController player;

    public GameObject bullet;

    float yPos;

    Rigidbody rb;

    int phase = 1;

    bool startShoot = false;

    void Awake()
    {
        yPos = transform.position.y;
        rb = GetComponent<Rigidbody>();
        indicatorLocal = ultiSphere.localPosition;
    }

    void Start()
    {
        player = ThirdPersonController.instance;
        StartCoroutine(Chase());
        InitAnimStates();
    }

    void Update()
    {
        if (Input.GetKeyDown(KeyCode.Z)) 
        {
            phase = 2;
        }
    }

    //Decision making is here
    IEnumerator Chase()
    {
        float timer = chaseDuration;
        while (timer > 0)
        {
            if (phase == 2)
            {
                rb.linearVelocity = vec3.Zero;
                SetState("Run", false);
                StartCoroutine(UltimateAttack());
                yield break;
            }
            SetState("Run", true);
            float dist = vec3.Distance(transform.position, player.transform.position);
            vec3 dir = (player.transform.position - transform.position) / dist;
            dir.y = 0;
            UpdateRotation(dir,rotationSpeed);
            rb.linearVelocity = dir * chaseSpeed * Time.deltaTime;
            timer -= Time.deltaTime;
            if (dist < basicAttackDistance)
            {
                rb.linearVelocity = vec3.Zero;
                SetState("Run", false);
                StartCoroutine(BasicAttack());
                yield break;
            }
            yield return null;
        }
        SetState("Run", false);
        StartCoroutine(DashAttack());
        rb.linearVelocity = vec3.Zero;
    }

    IEnumerator BasicAttack()
    {
        float timer = basicAttackDuration;
        float dist = 0;
        SetState("Attack1", true);
        while (timer > 0)
        {
            dist = vec3.Distance(transform.position, player.transform.position);
            vec3 dir = (player.transform.position - transform.position) / dist;
            dir.y = 0;
            if (timer > basicAttackDuration / 2)
            {
                UpdateRotation(dir, rotationSpeed);
            }
            timer -= Time.deltaTime;
            yield return null;
        }
        SetState("Attack1", false);
        if (dist > basicAttackDistance)
        {
            StartCoroutine(DashAttack());
            yield break;
        }
        else
        {
            StartCoroutine(Dodge());
        }
    }

    IEnumerator Dodge()
    {
        float timer = dodgeDuration;
        float dist = 0;
        vec3 targetPos = vec3.Zero;
        targetPos.y = yPos;
        vec3 startPos = transform.position;
        SetState("Dodge", true);
        while (timer > 0)
        {
            dist = vec3.Distance(startPos, targetPos);
            vec3 dir = (targetPos - startPos) / dist;
            UpdateRotation(dir, rotationSpeed);
            timer -= Time.deltaTime;
            vec3 pos = vec3.Lerp(startPos+dir* 5f, startPos, timer / dodgeDuration);
            transform.position = pos;
            yield return null;
        }
        SetState("Dodge", false);
        StartCoroutine(Chase());
    }

    IEnumerator DashAttack()
    {
        float chargeUpDuration = dashAttackDuration / 4f;
        float timer = chargeUpDuration;

        float dist = 0;
        vec3 dir = vec3.Zero;
        SetState("DashAttack", true);
        vec3 startPos = transform.localPosition;
        vec3 targetPos = vec3.Zero;
        animator.SetSpeed(0.5f);
        while (timer > 0)
        {
            targetPos = player.transform.position;
            targetPos.y = transform.position.y;
            dist = vec3.Distance(transform.position, targetPos);
            dir = (targetPos - transform.position) / dist;
            UpdateRotation(dir, rotationSpeed);
/*            if (timer < chargeUpDuration/2f)
            {
                animator.SetSpeed(0.5f);
            }
            else
            {

            }*/
            timer -= Time.deltaTime;
            yield return null;
        }
        animator.SetSpeed(1f);

        timer = chargeUpDuration;
        while (timer > 0)
        {
            vec3 pos = vec3.Lerp(targetPos + dir * 5f, startPos, timer / chargeUpDuration);
            transform.position = pos;
            timer -= Time.deltaTime;
            yield return null;
        }
        //Cooldown
        timer = chargeUpDuration;
        rb.linearVelocity = vec3.Zero;
        while (timer > 0)
        {
            timer -= Time.deltaTime;
            yield return null;
        }
        SetState("DashAttack", false);
        StartCoroutine(Chase());
    }

    IEnumerator SlamAttack()
    {
        float jumpDur = jumpAttackDuration * 0.3f;
        float startDur = jumpAttackDuration * 0.5f;
        float slamDur = jumpAttackDuration - jumpDur - startDur;

        vec3 startPos = transform.localPosition;
        startPos.y = 0;
        vec3 targetPos = player.transform.localPosition;
        targetPos.y = 0f;

        float dist = 0;
        vec3 dir = vec3.Zero;
        float timer = startDur;
        SetState("Jump", true);

        float indicatorY = ultiSphere.position.y;

        while (timer > 0)
        {
            targetPos = player.transform.localPosition;
            targetPos.y = 0f;
            dist = vec3.Distance(startPos, targetPos);
            dir = (targetPos - startPos) / dist;
            UpdateRotation(dir, rotationSpeed/2f);
            transform.position = vec3.Lerp(startPos, startPos + transform.back * dist + vec3.UnitY * 50f, (1 - timer / startDur) * 0.5f);

            vec3 indicatorPos = startPos + transform.back * dist;
            indicatorPos.y = indicatorY;
            ultiSphere.position = indicatorPos;
            timer -= Time.deltaTime;
            ultiSphere.localScale = vec3.Lerp(vec3.Ones * slamAttackRadius, vec3.Ones * 0.1f, timer / startDur);
            yield return null;
        }
        startPos = transform.localPosition;
        targetPos = startPos + transform.back * dist * 0.5f;
        targetPos.y = yPos;
        timer = jumpDur;
        while (timer > 0)
        {
            transform.localPosition = vec3.Lerp(startPos, targetPos, (1 - timer / jumpDur));
            vec3 indicatorPos = targetPos;
            indicatorPos.y = indicatorY;
            ultiSphere.position = indicatorPos;
            timer -= Time.deltaTime;
            yield return null;
        }
        ultiSphere.localPosition = indicatorLocal;
        ultiSphere.scale = vec3.Ones;
        transform.localPosition = targetPos;
        timer = slamDur;
        //Freeze
        while (timer > 0)
        {
            timer -= Time.deltaTime;
            yield return null;
        }
        StartCoroutine(ProjectileAttack());
    }

    IEnumerator UltimateAttack()
    {
        float timer = ultiChargeDuration;

        vec3 sphereScale = ultiSphere.localScale;

        while (timer > 0)
        {
            SetState("Idle", true);
            ultiSphere.localScale = vec3.Lerp(ultimateSize, sphereScale, timer / ultiChargeDuration);
            timer -= Time.deltaTime;
            yield return null;
        }
        timer = ultiExplodeDuration;
        //ACTUAL BOOM EXPANDS
        while (timer > 0)
        {
            SetState("Idle", true);
            timer -= Time.deltaTime;
            yield return null;
        }
        ultiSphere.localScale = sphereScale;
        //BOOM
        StartCoroutine(SlamAttack());
    }

    IEnumerator StartBullet(GameObject bullet)
    {
        while (!startShoot)
            yield return null;
        bullet.GetComponent<Rigidbody>().linearVelocity = bullet.transform.back * projectileSpeed;
    }

    IEnumerator ProjectileAttack()
    {
        int cycles = 4;

        int directions = 16;
        float angle = 360 / directions;
        float offset = 10f;


        float intervals = projectileAttackDuration / 2 / cycles;
        float timer;
        for (int i = 0; i < cycles; i++)
        {
            for (int d = 0; d < directions; d++)
            {
                timer = intervals / directions;
                vec3 rot = new vec3(0, glm.Radians(offset * i + angle * d), 0) ;
                GameObject obj = Instantiate(bullet, transform.localPosition + vec3.UnitY, rot);
                StartCoroutine(StartBullet(obj));
                obj.transform.position += obj.transform.back * 2f;
                timer = intervals/directions;
                while (timer > 0)
                {
                    timer -= Time.deltaTime;
                    yield return null;
                }
            }

            startShoot = true;
            timer = intervals;
            while (timer > 0)
            {
                vec3 dir = (player.transform.localPosition - transform.localPosition)/vec3.Distance(player.transform.localPosition, transform.localPosition);
                UpdateRotation(dir,rotationSpeed);
                SetState("Idle", true);
                timer -= Time.deltaTime;
                yield return null;
            }

            startShoot = false;
        }
        StartCoroutine(SlamAttack());
    }

    public void UpdateRotation(vec3 dir, float rotSpeed)
    {
        if (dir == vec3.Zero)
            return;
        float angle = (float)Math.Atan2(-dir.x, -dir.z);
        quat newQuat = glm.FromEulerToQuat(new vec3(0, angle, 0)).Normalized;
        quat oldQuat = glm.FromEulerToQuat(transform.localRotation).Normalized;

        // Interpolate using spherical linear interpolation (slerp)
        quat midQuat = quat.SLerp(oldQuat, newQuat, Time.deltaTime * rotSpeed);

        vec3 rot = ((vec3)midQuat.EulerAngles);

        if (rot != vec3.NaN)
        {
            bool isNan = false;
            foreach (float val in rot)
            {
                if (float.IsNaN(val))
                {
                    isNan = true;
                    break;
                }
            }
            if (!isNan)
            {
                transform.localRotation = rot;
            }
        }
    }

    void InitAnimStates()
    {
        animationManager = new AnimationStateMachine(animator);

        //Highest Precedence
        AnimationState death = animationManager.GetState("Death");
        AnimationState stun = animationManager.GetState("Stun");
        AnimationState falling = animationManager.GetState("Falling");
        AnimationState jump = animationManager.GetState("Jump");
        AnimationState overdrive = animationManager.GetState("Overdrive");
        AnimationState dashAttack = animationManager.GetState("DashAttack");
        dashAttack.speed = 2f;

        AnimationState dodge = animationManager.GetState("Dodge");
        AnimationState attack1 = animationManager.GetState("Attack1");
        AnimationState attack2 = animationManager.GetState("Attack2");
        AnimationState attack3 = animationManager.GetState("Attack3");
        AnimationState sprint = animationManager.GetState("Sprint");
        AnimationState run = animationManager.GetState("Run");
        //Lowest Precedence

        attack1.speed = 1.2f;
    }

    bool GetState(string stateName)
    {
        return animationManager.GetState(stateName).state;
    }
    void SetState(string stateName, bool value)
    {
        animationManager.GetState(stateName).state = value;
        animationManager.UpdateState();
    }
}