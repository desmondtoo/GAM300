﻿using System.Collections;
using System.Collections.Generic;
using BeanFactory;
using GlmSharp;
using System;
using System.Threading;
using System.Resources;

public class Enemy : Script
{
    public float moveSpeed = 2f;
    public float chaseDistance = 10f;
    public float attackDistance = 3f;
    public float attackCooldown = 0f;
    public Transform player;
    public int count;

    public float maxHealth = 3f;
    public float currentHealth;
    public bool isDead = false;
    // HealthBar
    public Transform hpBar;
    public bool inRange = false;
    public bool back = false;

    public Transform meleeEnemyPos;
    public Transform modelOffset;
    public float RotationSpeed = 6f;

    AnimationStateMachine animationManager;
    public Animator animator;
    public int state;//Example 1 is walk, 2 is attack, 3 is idle etc.
    public bool startDeathAnimationCountdown = false;
    float animationTimer = 2.5f;
    public float currentAnimationTimer;
    private float currentDeathAnimationTimer = 2.8f;
    private Coroutine damagedCoroutine = null;

    public float testingValue = 0f;

    //public GameObject spawnObject;
    public GameObject attackTrigger;
    public Transform parentTransform;
    Rigidbody rb;
    public bool touchingPlayer = false;

    public bool isAttacking = false;
    public bool isAttackCooldown = false;
    float attackTimer = 1f;
    float currentAttackTimer;
    float currentAttackBuffer;
    public float attackCooldownTimer = 1f;
    public float currentAttackCooldownTimer;

    // NavMesh stuff
    public float duration = 0.2f;
    public float timer = 0f;
    public bool newRequest = false;
    private bool attackOnce = false;

    public bool isStunned;
    public float stunDuration = 0.5f;
    public float currentStunDuration;

    private NavMeshAgent mNavMeshAgent;

    // Staggering stuff
    public float staggerCooldown = 5f;
    public float staggerTimer = 5f;

    //audio
    public bool playOnce = true;
    public bool alertedOnce = true;
    int alertedRotation = 0;
    PlayerAudioManager playerSounds;
    public EnemyAudioManager enemySounds;

    private vec3 initialPosition;

    void Start()
    {
        if (enemySounds == null)
        {
            Console.WriteLine("Missing Enemy audio manager in enemy");
            return;
        }
        initialPosition = transform.position;
        mNavMeshAgent = GetComponent<NavMeshAgent>();
        playerSounds = PlayerAudioManager.instance;
        //enemySounds = EnemyAudioManager.instance;
        playOnce = true;
        currentHealth = maxHealth;
        state = 0;//start with idle state
        currentStunDuration = stunDuration;
        rb = GetComponent<Rigidbody>();
        InitAnimStates();
    }
   

    void Update()
    {
        if (player == null)
        {
            //SetState("Idle", true);
            return;
        }

        //debugging state
        //Console.WriteLine(state);

        //death animation timer
/*        if (startDeathAnimationCountdown)
        {
            SetEnabled(GetComponent<Rigidbody>(), false);
            Console.WriteLine("Progress: " + animator.GetProgress());
            currentDeathAnimationTimer -= Time.deltaTime;
            if (animator.GetState() == "Death" && animator.GetProgress() > 0.4f)
            {
                animator.SetSpeed(0f);
                currentDeathAnimationTimer = animationTimer;
                startDeathAnimationCountdown = false;
                //animator.Pause();//pause the death animation to prevent it from returning to idle animation
                gameObject.SetActive(false);
                return;
                //Respawn();
                //SceneManager.LoadScene("LevelPlay2");
            }
        }*/

        if (isDead)
        {
            if (attackTrigger != null)
            {
                attackTrigger.SetActive(false);
            }
            return;
        }
        if(isStunned)
        {
            state = 4;//set to stun state
        }
        if(state == 2)
        {
            //Console.WriteLine("Progress: " + animator.GetProgress());
            isAttacking = true;
            if(isAttacking && !isAttackCooldown)
            {
                currentAttackTimer += Time.deltaTime;
                currentAttackBuffer += Time.deltaTime;
                //if (currentAttackBuffer >= 0.6f) // So that the attack is not instantaneous
                //if(animator.GetProgress() >= 2.786f && !attackOnce)
                bool animatorCheck = animator.GetState() == "Attack" && animator.GetProgress() > 0.0f && animator.GetProgress() < 0.1f;
                if(animatorCheck && !attackOnce)
                {
                    attackOnce = true;
                    if (attackTrigger != null)
                    {
                        attackTrigger.SetActive(true);
                        attackTrigger.GetComponent<Rigidbody>().linearVelocity = new vec3(modelOffset.back * 0.6f);
                        attackTrigger.transform.localPosition = new vec3(transform.localPosition + modelOffset.forward * 0.6f);
                        attackTrigger.transform.rotation = new vec3(modelOffset.rotation);
                    }
                    currentAttackBuffer = 0f;
                }
                
                //AudioManager.instance.meleeEnemyAttack.Play();
                if (currentAttackTimer >= attackTimer)
                {
                    isAttacking = false;
                    isAttackCooldown = true;
                    //SetState("Attack", false);
                    currentAttackTimer = 0f;
                }
            }
            if(isAttackCooldown)
            {
                currentAttackCooldownTimer += Time.deltaTime;
                if (attackTrigger != null)
                {
                    //attackTrigger.SetActive(false);
                    attackTrigger.transform.position = vec3.Zero;
                }

                if (currentAttackCooldownTimer >= attackCooldownTimer)
                {
                    isAttackCooldown = false;
                    currentAttackCooldownTimer = 0f;
                }
            }

            //if(animator.GetProgress() < 2.7f)
            if(animator.GetProgress() > 0.5f)
            {
                attackOnce = false;
                isAttackCooldown = false;
                attackTrigger.transform.position = vec3.Zero;
            }
           
        }
        else if (state != 2)
        {
            attackTrigger.SetActive(false);
            isAttacking = false;
            isAttackCooldown = false;
            currentAttackCooldownTimer = 0f;
            currentAnimationTimer = 0f;
        }


        vec3 direction = player.position - transform.position;
        direction = direction.NormalizedSafe;
        direction.y = 0f;

        vec3 dir = initialPosition - transform.position;
        dir = dir.NormalizedSafe;
        dir.y = 0f;

        if (!isDead)
        {
            switch (state)
            {
                //idle state
                case 0:
                    //idle animation
                    playOnce = true;//reset ability to play audio
                    alertedOnce = true;
                    SetState("Attack", false);
                    SetState("Idle", true);

                    float dist = vec2.Distance(initialPosition.xz, transform.position.xz);
                    //Console.WriteLine("DistanceIdle: " + dist);
                    //attackTrigger.SetActive(false);
                    //player detection
                    if (vec3.Distance(player.localPosition, transform.localPosition) <= chaseDistance && mNavMeshAgent.FindPath(player.position))
                    {
                        if (animationManager.GetState("Idle").state)
                        {
                            SetState("Idle", false);
                        }
                        //change to chase state
                        state = 1;
                    }
                    else if (dist >= 1.5f)
                    {
                        if (animationManager.GetState("Idle").state)
                        {
                            SetState("Idle", false);
                        }
                        // Change to go back initial position state
                        state = 5;
                    }

                    rb.linearVelocity = vec3.Zero;
                    break;
                //chase state
                case 1:
                    //Console.WriteLine("Chase");
                    SetState("Run", true);
                    playOnce = true;//reset ability to play audio
                   
                    //attackTrigger.SetActive(false);
                    //change to attack state once it has reach it is in range

                    staggerTimer += Time.deltaTime; // Start counting stagger timer

                    if (alertedOnce)
                    {
                        Random rd = new Random();
                        alertedRotation = rd.Next(0, 2);
                        alertedOnce = false;

                        switch (alertedRotation)
                        {
                            case 0:
                                enemySounds.EnemyAlerted1.Play();
                                break;
                            case 1:
                                enemySounds.EnemyAlerted2.Play();
                                break;
                            case 2:
                                enemySounds.EnemyAlerted3.Play();
                                break;

                        }
                    }

                    if (vec3.Distance(player.localPosition, transform.localPosition) <= attackDistance)
                    {
                        state = 2;
                        attackCooldown = 0f;
                    }
                    //return to its starting position if player is far from its chaseDistance
                    if (vec3.Distance(player.localPosition, transform.localPosition) > chaseDistance)
                    {
                        if (animationManager.GetState("Run").state)
                        {
                            SetState("Run", false);
                        }
                        //return back to its previous position state
                        LookAt(direction);
                        mNavMeshAgent.FindPath(initialPosition);
                        state = 0;
                    }
                    else
                    {
                        if (mNavMeshAgent != null) // Use navmesh if is navmesh agent
                        {
                            LookAt(direction);
                            mNavMeshAgent.FindPath(player.position);
                        }
                        else // Default
                        {
                            LookAt(direction);
                            rb.linearVelocity = direction * moveSpeed;
                        }
                    }

                    break;
                //attack state
                case 2:
                    //Console.WriteLine("Attack");
                    SetState("Attack", true); //attack animation
                    staggerTimer += Time.deltaTime; // Start counting stagger timer
                    rb.linearVelocity = vec3.Zero;

                    if (mNavMeshAgent != null) // Use navmesh if is navmesh agent
                    {
                        //mNavMeshAgent.FindPath(transform.position); // Bean: this is causing the teleporting issue
                        mNavMeshAgent.ResetPath();
                    }

                    if (playOnce)
                    {
                        playOnce = false;
                        enemySounds.MeleeEnemyAttack.Play();
                    }

                    if(!isAttacking)
                    {
                        
                        //change to chase state once player has reach out of range
                        if (vec3.Distance(player.localPosition, transform.localPosition) > attackDistance)
                        {
                            if (animationManager.GetState("Attack").state)
                            {
                                SetState("Attack", false);
                            }
                            state = 1;
                        }
                    }
                    else
                    {
                        LookAt(direction);
                    }

                    break;

                //death state
                case 3:
                    //Console.WriteLine("Death");
                    SetState("Death", true);
                    animationManager.UpdateState();

                    break;
                //stun state
                case 4:
                    //Console.WriteLine("Stunned");
                    SetState("Stun", true);
                    //attackTrigger.SetActive(false);
                    GetComponent<Rigidbody>().linearVelocity = vec3.Zero;
                    if (mNavMeshAgent != null) // Use navmesh if is navmesh agent
                    {
                        //mNavMeshAgent.FindPath(transform.position); // Bean: this is causing the teleporting issue
                        mNavMeshAgent.ResetPath();
                    }
                   

                    currentStunDuration -= Time.deltaTime;
                    if (currentStunDuration <= 0)
                    {
                        if (animationManager.GetState("Stun").state)
                        {
                            SetState("Stun", false);
                        }
                        isStunned = false;
                        state = 0;//reset back to idle state
                        currentStunDuration = stunDuration;
                    }
                    //animationManager.UpdateState();
                    break;
                // Go back to initial position state
                case 5:
                    SetState("Run", true);
                    float distance = vec3.Distance(initialPosition, transform.position);
                    if (distance < 1.5f)
                    {
                        if (animationManager.GetState("Run").state)
                        {
                            SetState("Run", false);
                        }
                        state = 0; // Go back to idle state
                    }
                    else if (vec3.Distance(player.localPosition, transform.localPosition) <= chaseDistance && mNavMeshAgent.FindPath(player.position))
                    {
                        state = 1;
                    }
                    else
                    {
                        if (mNavMeshAgent != null) // Use navmesh if is navmesh agent
                        {
                            LookAt(dir);
                            mNavMeshAgent.FindPath(initialPosition);
                        }
                        else // Default
                        {
                            LookAt(dir);
                            GetComponent<Rigidbody>().linearVelocity = dir * moveSpeed;
                        }
                    }

                    break;

            }
        }

        //timer += Time.deltaTime;

        //needed for the animation to change
        animationManager.UpdateState();
    }

    void LookAt(vec3 dir)
    {
        if (dir == vec3.Zero)
            return;
        float angle = (float)Math.Atan2(dir.x, dir.z);
        quat newQuat = glm.FromEulerToQuat(new vec3(0, angle, 0)).Normalized;
        quat oldQuat = glm.FromEulerToQuat(transform.rotation).Normalized;

        // Interpolate using spherical linear interpolation (slerp)
        quat midQuat = quat.SLerp(oldQuat, newQuat, Time.deltaTime * RotationSpeed);

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
                transform.rotation = rot;
            }
        }
    }

    // Start is called before the first frame update
    void InitAnimStates()
    {
        animationManager = new AnimationStateMachine(animator);
        currentAnimationTimer = animationTimer;

        //Highest Precedence
        AnimationState death = animationManager.GetState("Death");
        AnimationState stun = animationManager.GetState("Stun");
        AnimationState attack = animationManager.GetState("Attack");
        AnimationState walk = animationManager.GetState("Walk");
        AnimationState run = animationManager.GetState("Run");
        AnimationState idle = animationManager.GetState("Idle");
        //Lowest Precedence

        stun.SetConditionals(false, death);
        attack.SetConditionals(false, death, stun);
        attack.speed = 1.5f;
        walk.SetConditionals(true, walk);
        walk.SetConditionals(false, attack, death, stun);
        run.SetConditionals(false, attack, death, stun);
    }

    bool GetState(string stateName)
    {
        return animationManager.GetState(stateName).state;
    }
    void SetState(string stateName, bool value)
    {
        animationManager.GetState(stateName).state = value;
    }

    void TakeDamage(int amount)
    {
        if(!isDead)
        {
            ThirdPersonCamera.instance.ShakeCamera(CombatManager.instance.hitShakeMag, CombatManager.instance.hitShakeDur);
            ThirdPersonCamera.instance.SetFOV(-CombatManager.instance.hitShakeMag * 150, CombatManager.instance.hitShakeDur * 4);
            enemySounds.EnemyHit.Play();
            enemySounds.MeleeEnemyInjured.Play();
            currentHealth -= amount;
            vec3 hpScale = hpBar.localScale;
            hpScale.x = currentHealth / maxHealth;
            hpBar.localScale = hpScale;
            //Console.WriteLine("HHHH: " + hpBar.localScale.x + " " + hpBar.localScale.y + " " + hpBar.localScale.z);
            CombatManager.instance.SpawnHitEffect(transform);
        }

        //set particle transform to enemy position
        if (currentHealth <= 0)
        {
            CombatManager.instance.SpawnHitEffect2(transform);
            currentHealth = 0;
            hpBar.gameObject.SetActive(false);
            isDead = true;
            StartCoroutine(StartDeath());
            //Console.WriteLine("EnemyDead");
            SetState("Death", true);
            animationManager.UpdateState();
            startDeathAnimationCountdown = true;
            enemySounds.MeleeEnemyDie.Play();

            //if (spawnObject != null)
            //    spawnObject.SetActive(true);
            //Destroy(gameObject);
        }
    }

    IEnumerator StartDeath()
    {
        rb.linearVelocity = vec3.Zero;
        SetEnabled(rb,false);
        float timer = 0;
        yield return new WaitForSeconds(2f);
        animator.SetSpeed(0);
        yield return new WaitForSeconds(1f);
        gameObject.SetActive(false);
    }

    void Exit()
    {

    }

    void OnTriggerEnter(PhysicsComponent other)
    {
        //check if the rigidbody belongs to a game object called PlayerWeaponCollider
        if (GetTag(other) == "PlayerAttack" && !isDead)
        {
            Transform otherT = other.gameObject.GetComponent<Transform>();
            vec3 dir = otherT.forward;
            dir = dir.NormalizedSafe;
            if (staggerTimer >= staggerCooldown)
            {
                isStunned = true;
                staggerTimer = 0f;
            }

            if (damagedCoroutine != null)
            {
                StopCoroutine(damagedCoroutine);
            }
            damagedCoroutine = StartCoroutine(Damaged(.5f, dir * 6));

            if (ThirdPersonController.instance.currentlyOverdriven == true || ThirdPersonController.instance._isDashAttacking)
            {
                TakeDamage(2);
            }
            else
            {
                TakeDamage(1);

                if (ThirdPersonController.instance.isOverdriveEnabled == true)
                {
                    //This allows the player to charge his overdrive while ONLY NOT BEING IN OVERDRIVE
                    if (ThirdPersonController.instance.currentOverdriveCharge >= ThirdPersonController.instance.maxOverdriveCharge)
                    {
                        ThirdPersonController.instance.currentOverdriveCharge = ThirdPersonController.instance.maxOverdriveCharge;
                        ThirdPersonController.instance.UpdateOverdriveBar();

                        if (ThirdPersonController.instance.playOverdrivePowerUpOnce == true)
                        {
                            ThirdPersonController.instance.playOverdrivePowerUpOnce = false;
                            playerSounds.PowerUp.Play();
                            ThirdPersonController.instance.overDriveUI.gameObject.SetActive(true);
                        }
                    }
                    else
                    {
                        ThirdPersonController.instance.currentOverdriveCharge++;
                        ThirdPersonController.instance.UpdateOverdriveBar();

                        if (ThirdPersonController.instance.playOverdrivePowerUpOnce == true && ThirdPersonController.instance.currentOverdriveCharge >= ThirdPersonController.instance.maxOverdriveCharge)
                        {
                            ThirdPersonController.instance.playOverdrivePowerUpOnce = false;
                            playerSounds.PowerUp.Play();
                            ThirdPersonController.instance.overDriveUI.gameObject.SetActive(true);
                        }
                    }
                }
            }
        }
    }

    IEnumerator Damaged(float duration, vec3 knockback)
    {
        duration /= 2;
        float startDuration = duration;
        while (duration > 0)
        {
            rb.linearVelocity = new vec3(knockback * (duration / startDuration));
            duration -= Time.deltaTime;
            yield return new WaitForSeconds(Time.deltaTime);
        }
        duration = startDuration;
        while (duration > 0)
        {
            float val = glm.Radians(-45f);
            duration -= Time.deltaTime;
            yield return new WaitForSeconds(Time.deltaTime);
        }
    }

    void OnCollisionEnter(PhysicsComponent component)
    {
        //if (GetTag(component) == "PlayerCollider")
        //{
        //    Console.WriteLine("I have been attacked!");
        //    currentHealth -= 1;
        //    if (currentHealth <= 0)
        //        Destroy(gameObject);
        //}
    }
}

