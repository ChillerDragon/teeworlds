from datatypes import *

class Pickup(Struct):
	def __init__(self, name="", respawntime=15, spawndelay=0):
		Struct.__init__(self, "CDataPickupspec")
		self.name = String(name)
		self.respawntime = Int(respawntime)
		self.spawndelay = Int(spawndelay)

class AnimKeyframe(Struct):
	def __init__(self, time=0, x=0, y=0, angle=0):
		Struct.__init__(self, "CAnimKeyframe")
		self.time = Float(time)
		self.x = Float(x)
		self.y = Float(y)
		self.angle = Float(angle)

class AnimSequence(Struct):
	def __init__(self):
		Struct.__init__(self, "CAnimSequence")
		self.frames = Array(AnimKeyframe())

class Animation(Struct):
	def __init__(self, name=""):
		Struct.__init__(self, "CAnimation")
		self.name = String(name)
		self.body = AnimSequence()
		self.back_foot = AnimSequence()
		self.front_foot = AnimSequence()
		self.attach = AnimSequence()

class WeaponSpec(Struct):
	def __init__(self, container=None, name=""):
		Struct.__init__(self, "CDataWeaponspec")
		self.name = String(name)
		self.visual_size = Int(96)

		self.firedelay = Int(500)
		self.maxammo = Int(10)
		self.ammoregentime = Int(0)
		self.damage = Int(1)

		self.offsetx = Float(0)
		self.offsety = Float(0)
		self.muzzleoffsetx = Float(0)
		self.muzzleoffsety = Float(0)
		self.muzzleduration = Float(5)

class Weapon_Hammer(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataWeaponspecHammer")
		self.base = Pointer(WeaponSpec, WeaponSpec())

class Weapon_Gun(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataWeaponspecGun")
		self.base = Pointer(WeaponSpec, WeaponSpec())
		self.curvature = Float(1.25)
		self.speed = Float(2200)
		self.lifetime = Float(2.0)

class Weapon_Shotgun(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataWeaponspecShotgun")
		self.base = Pointer(WeaponSpec, WeaponSpec())
		self.curvature = Float(1.25)
		self.speed = Float(2200)
		self.speeddiff = Float(0.8)
		self.lifetime = Float(0.25)

class Weapon_Grenade(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataWeaponspecGrenade")
		self.base = Pointer(WeaponSpec, WeaponSpec())
		self.curvature = Float(7.0)
		self.speed = Float(1000)
		self.lifetime = Float(2.0)

class Weapon_Laser(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataWeaponspecLaser")
		self.base = Pointer(WeaponSpec, WeaponSpec())
		self.reach = Float(800.0)
		self.bounce_delay = Int(150)
		self.bounce_num = Int(1)
		self.bounce_cost = Float(0)

class Weapon_Ninja(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataWeaponspecNinja")
		self.base = Pointer(WeaponSpec, WeaponSpec())
		self.duration = Int(15000)
		self.movetime = Int(200)
		self.velocity = Int(50)

class Weapons(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataWeaponspecs")
		self.hammer = Weapon_Hammer()
		self.gun = Weapon_Gun()
		self.shotgun = Weapon_Shotgun()
		self.grenade = Weapon_Grenade()
		self.laser = Weapon_Laser()
		self.ninja = Weapon_Ninja()
		self.id = Array(WeaponSpec())

class Explosion(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataExplosion")
		self.radius = Float(135)
		self.max_force = Float(12)

class DataContainer(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataContainer")
		self.pickups = Array(Pickup())
		self.animations = Array(Animation())
		self.weapons = Weapons()
		self.explosion = Explosion()

def FileList(format, num):
	return [format%(x+1) for x in range(0,num)]

container = DataContainer()

container.pickups.Add(Pickup("health"))
container.pickups.Add(Pickup("armor"))
container.pickups.Add(Pickup("grenade"))
container.pickups.Add(Pickup("shotgun"))
container.pickups.Add(Pickup("laser"))
container.pickups.Add(Pickup("ninja", 90, 90))
container.pickups.Add(Pickup("gun"))
container.pickups.Add(Pickup("hammer"))

anim = Animation("base")
anim.body.frames.Add(AnimKeyframe(0, 0, -4, 0))
anim.back_foot.frames.Add(AnimKeyframe(0, 0, 10, 0))
anim.front_foot.frames.Add(AnimKeyframe(0, 0, 10, 0))
container.animations.Add(anim)

anim = Animation("idle")
anim.back_foot.frames.Add(AnimKeyframe(0, -7, 0, 0))
anim.front_foot.frames.Add(AnimKeyframe(0, 7, 0, 0))
container.animations.Add(anim)

anim = Animation("inair")
anim.back_foot.frames.Add(AnimKeyframe(0, -3, 0, -0.1))
anim.front_foot.frames.Add(AnimKeyframe(0, 3, 0, -0.1))
container.animations.Add(anim)

anim = Animation("walk")
anim.body.frames.Add(AnimKeyframe(0.0, 0, 0, 0))
anim.body.frames.Add(AnimKeyframe(0.2, 0,-1, 0))
anim.body.frames.Add(AnimKeyframe(0.4, 0, 0, 0))
anim.body.frames.Add(AnimKeyframe(0.6, 0, 0, 0))
anim.body.frames.Add(AnimKeyframe(0.8, 0,-1, 0))
anim.body.frames.Add(AnimKeyframe(1.0, 0, 0, 0))

anim.back_foot.frames.Add(AnimKeyframe(0.0, 8, 0, 0))
anim.back_foot.frames.Add(AnimKeyframe(0.2, -8, 0, 0))
anim.back_foot.frames.Add(AnimKeyframe(0.4,-10,-4, 0.2))
anim.back_foot.frames.Add(AnimKeyframe(0.6, -8,-8, 0.3))
anim.back_foot.frames.Add(AnimKeyframe(0.8, 4,-4,-0.2))
anim.back_foot.frames.Add(AnimKeyframe(1.0, 8, 0, 0))

anim.front_foot.frames.Add(AnimKeyframe(0.0,-10,-4, 0.2))
anim.front_foot.frames.Add(AnimKeyframe(0.2, -8,-8, 0.3))
anim.front_foot.frames.Add(AnimKeyframe(0.4, 4,-4,-0.2))
anim.front_foot.frames.Add(AnimKeyframe(0.6, 8, 0, 0))
anim.front_foot.frames.Add(AnimKeyframe(0.8, 8, 0, 0))
anim.front_foot.frames.Add(AnimKeyframe(1.0,-10,-4, 0.2))
container.animations.Add(anim)

anim = Animation("hammer_swing")
anim.attach.frames.Add(AnimKeyframe(0.0, 0, 0, -0.10))
anim.attach.frames.Add(AnimKeyframe(0.3, 0, 0, 0.25))
anim.attach.frames.Add(AnimKeyframe(0.4, 0, 0, 0.30))
anim.attach.frames.Add(AnimKeyframe(0.5, 0, 0, 0.25))
anim.attach.frames.Add(AnimKeyframe(1.0, 0, 0, -0.10))
container.animations.Add(anim)

anim = Animation("ninja_swing")
anim.attach.frames.Add(AnimKeyframe(0.00, 0, 0, -0.25))
anim.attach.frames.Add(AnimKeyframe(0.10, 0, 0, -0.05))
anim.attach.frames.Add(AnimKeyframe(0.15, 0, 0, 0.35))
anim.attach.frames.Add(AnimKeyframe(0.42, 0, 0, 0.40))
anim.attach.frames.Add(AnimKeyframe(0.50, 0, 0, 0.35))
anim.attach.frames.Add(AnimKeyframe(1.00, 0, 0, -0.25))
container.animations.Add(anim)

weapon = WeaponSpec(container, "hammer")
weapon.firedelay.Set(125)
weapon.damage.Set(3)
weapon.visual_size.Set(96)
weapon.offsetx.Set(4)
weapon.offsety.Set(-20)
container.weapons.hammer.base.Set(weapon)
container.weapons.id.Add(weapon)

weapon = WeaponSpec(container, "gun")
weapon.firedelay.Set(125)
weapon.damage.Set(1)
weapon.ammoregentime.Set(500)
weapon.visual_size.Set(64)
weapon.offsetx.Set(32)
weapon.offsety.Set(0)
weapon.muzzleoffsetx.Set(50)
weapon.muzzleoffsety.Set(6)
container.weapons.gun.base.Set(weapon)
container.weapons.id.Add(weapon)

weapon = WeaponSpec(container, "shotgun")
weapon.firedelay.Set(500)
weapon.damage.Set(1)
weapon.visual_size.Set(96)
weapon.offsetx.Set(24)
weapon.offsety.Set(-2)
weapon.muzzleoffsetx.Set(70)
weapon.muzzleoffsety.Set(6)
container.weapons.shotgun.base.Set(weapon)
container.weapons.id.Add(weapon)

weapon = WeaponSpec(container, "grenade")
weapon.firedelay.Set(500) # TODO: fix this
weapon.damage.Set(6)
weapon.visual_size.Set(96)
weapon.offsetx.Set(24)
weapon.offsety.Set(-2)
container.weapons.grenade.base.Set(weapon)
container.weapons.id.Add(weapon)

weapon = WeaponSpec(container, "laser")
weapon.firedelay.Set(800)
weapon.damage.Set(5)
weapon.visual_size.Set(92)
weapon.offsetx.Set(24)
weapon.offsety.Set(-2)
container.weapons.laser.base.Set(weapon)
container.weapons.id.Add(weapon)

weapon = WeaponSpec(container, "ninja")
weapon.firedelay.Set(800)
weapon.damage.Set(9)
weapon.visual_size.Set(96)
weapon.offsetx.Set(0)
weapon.offsety.Set(0)
weapon.muzzleoffsetx.Set(40)
weapon.muzzleoffsety.Set(-4)
container.weapons.ninja.base.Set(weapon)
container.weapons.id.Add(weapon)
