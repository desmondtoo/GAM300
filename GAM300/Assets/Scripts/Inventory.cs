﻿using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using BeanFactory;
using GlmSharp;


public class Inventory : Script
{
    public static Inventory instance;
    public ThirdPersonController thirdPersonController;

    private int _healthPackCount;
    private int _staminaPackCount;
    private int _maxStatPackCount;

    public int healthPackCount
    {
        get
        {
            return _healthPackCount;
        }
        set
        {
            _healthPackCount = value;
            InstanceData.SaveData("HealthPack", healthPackCount);
        }
    }
    public int staminaPackCount
    {
        get
        {
            return _staminaPackCount;
        }
        set
        {
            _staminaPackCount = value;
            InstanceData.SaveData("StaminaPack", staminaPackCount);
        }
    }

    public int maxStatPackCount
    {
        get
        {
            return _maxStatPackCount;
        }
        set
        {
            _maxStatPackCount = value;
            InstanceData.SaveData("MaxStatPack", maxStatPackCount);
        }
    }
    
    public float staminaPackValue = 100f;
    public float healthPackHealValue = 5f;

    PlayerAudioManager playerSounds;

    public TextRenderer stamCounter;
    public TextRenderer healthCounter;
    public TextRenderer maxPackCounter;

    public SpriteRenderer dashIcon;
    public SpriteRenderer dodgeIcon;
    public SpriteRenderer sprintIcon;

    void Awake()
    {
        //to fix issue where this reference is missing in the scene
        thirdPersonController = gameObject.GetComponent<ThirdPersonController>();

        if (instance != null)
        {
            //Debug.LogError("More than one AudioManager in the scene.");
        }
        else
        {
            instance = this;
        }
    }

    void Start()
    {
        playerSounds = PlayerAudioManager.instance;

        if (sprintIcon == null)
        {
            Console.WriteLine("Missing sprintIcon reference in Inventory script");
        }
        if (dodgeIcon == null)
        {
            Console.WriteLine("Missing dodgeIcon reference in Inventory script");
        }
        if (dashIcon == null)
        {
            Console.WriteLine("Missing dashIcon reference in Inventory script");
        }

        if (stamCounter == null)
        {
            Console.WriteLine("Missing stamcounter reference in Inventory script");
        }
        if (healthCounter == null)
        {
            Console.WriteLine("Missing healthCounter reference in Inventory script");
        }
        if (maxPackCounter == null)
        {
            Console.WriteLine("Missing maxPackCounter reference in Inventory script");
        }
        if (thirdPersonController == null)
        {
            Console.WriteLine("Missing thirdPersonController reference in Inventory script");
        }

        healthPackCount = (int)InstanceData.GetData("HealthPack");
        staminaPackCount = (int)InstanceData.GetData("StaminaPack");
        maxStatPackCount = (int)InstanceData.GetData("MaxStatPack");
    }

    void Update()
    {
        thePacks();
        changeAlphaIcon();
        //theCheats();

        stamCounter.text = staminaPackCount.ToString();
        healthCounter.text = healthPackCount.ToString();
        maxPackCounter.text = maxStatPackCount.ToString();
    }

    //(CHEAT) Adds one of eah pack count.
    void theCheats()
    {
        healthPackCount++;
        staminaPackCount++;
        maxStatPackCount++;
    }

    //this enables to change alpha values of ability icons
    void changeAlphaIcon()
    {
        //this changes the alpha of dodgeIcon
        if (thirdPersonController.startDodgeCooldown == true)
        {
            dodgeIcon.alpha = 0.5f;
        }
        else
        {
            dodgeIcon.alpha = 1f;
        }

        //this changes the alpha of dashIcon
        if (thirdPersonController.startDashCooldown == true)
        {
            dashIcon.alpha = 0.5f;
        }
        else
        {
            dashIcon.alpha = 1f;
        }

        //this changes the alpha of dashIcon
        if (thirdPersonController._isSprinting == true)
        {
            sprintIcon.alpha = 0.5f;
        }
        else
        {
            sprintIcon.alpha = 1f;
        }
    }

    IEnumerator delayRegenVFX()
    {
        ThirdPersonController.instance.healthRegenEnabled = true;
        yield return new WaitForSeconds(2f);
        ThirdPersonController.instance.healthRegenEnabled = false;
    }

    IEnumerator delayStaminaRegenVFX()
    {
        ThirdPersonController.instance.playerStaminaVFX.SetActive(true);
        yield return new WaitForSeconds(2f);
        ThirdPersonController.instance.playerStaminaVFX.SetActive(false);
    }

    IEnumerator delayMaxRegenVFX()
    {
        ThirdPersonController.instance.playerMaxVFX.SetActive(true);
        yield return new WaitForSeconds(2f);
        ThirdPersonController.instance.playerMaxVFX.SetActive(false);
    }

    public void useStaminaForScript()
    {
        StartCoroutine(delayStaminaRegenVFX());
    }

    void thePacks()
    {
        //HEALTH PACK - RESTORES HEALTH
        if (healthPackCount > 0 && Input.GetKeyDown(KeyCode.D1))
        {
            StartCoroutine(delayRegenVFX());
            healthPackCount -= 1;
            Console.WriteLine("Use Health Pack");
            playerSounds.UseItem.Play();
            thirdPersonController.HealHealth(healthPackHealValue);
            thirdPersonController.UpdatehealthBar();
        }

        //STAMINA PACK - RESTORE STAMINA
        if (staminaPackCount > 0 && Input.GetKeyDown(KeyCode.D2))
        {
            StartCoroutine(delayStaminaRegenVFX());
            staminaPackCount -= 1;
            Console.WriteLine("Use Stamina Pack");
            playerSounds.UseItem.Play();
            thirdPersonController.restoreStamina(staminaPackValue);
            thirdPersonController.UpdateStaminaBar();

        }

        //MAXSTAT PACK - RESTORE HEALTH AND STAMINA
        if (maxStatPackCount > 0 && Input.GetKeyDown(KeyCode.D3))
        {
            StartCoroutine(delayMaxRegenVFX());
            maxStatPackCount -= 1;
            Console.WriteLine("Use MaxStat Pack");
            playerSounds.UseItem.Play();
            thirdPersonController.HealHealth(thirdPersonController.maxHealth);
            thirdPersonController.UpdatehealthBar();
            thirdPersonController.restoreStamina(thirdPersonController.maxStamina);
            thirdPersonController.UpdateStaminaBar();
        }

        //Sound feedback when not enough items
        if ((maxStatPackCount <= 0 && Input.GetKeyDown(KeyCode.D3) || staminaPackCount <= 0 && Input.GetKeyDown(KeyCode.D2)) || healthPackCount <= 0 && Input.GetKeyDown(KeyCode.D1))
        {
            AudioManager.instance.notEnoughItems.Play();
        }
    }

}
