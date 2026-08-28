import math
import numpy as np
import pygame
import pymunk.pygame_util
from pymunk.constraints import DampedRotarySpring, PivotJoint, RotaryLimitJoint
import torch


pygame.init()
pymunk.pygame_util.positive_y_is_up = False


class MuscleJoint:
    def __init__(self, root, body_a, body_b, max_force = 1000, max_angle = (-180, 180)):
        self.root = root
        
        self.body_a = body_a
        self.body_b = body_b
        
        self.energy = 1.0
        self.spend_rate = 0.2
        self.recovery_rate = 0.05
        self._peak_rate = 0.2
        self._restore_rate = 0.05
        
        self.torque = 0.0
        self._max_p = max_force * 10000
        
        anchor_pos = (
            (self.body_a.position.x + self.body_b.position.x) / 2,
            (self.body_a.position.y + self.body_b.position.y) / 2
        )
        
        self.pivot = PivotJoint(self.body_a, self.body_b, anchor_pos)
        self.pivot.collide_bodies = False
        self.root.space.add(self.pivot)
        
        self.motor = DampedRotarySpring(self.body_a, self.body_b, 0, 0, 0)
        self.motor.max_force = max_force * 1e9
        self.root.space.add(self.motor)
        
        max_angle_rad = np.array(max_angle) * math.pi / 180
        
        self.limit = RotaryLimitJoint(self.body_a, self.body_b, max_angle_rad[0], max_angle_rad[1])
        self.root.space.add(self.limit)
    
    
    def remove(self):
        self.root.space.remove(self.pivot, self.motor, self.limit)
    
    
    def setTorque(self, target_angle, torque = 1.0):
        self.torque = torque#np.clip(torque * (1 - math.exp(-self.energy * 4)), 0, 1)
        self.spend_rate = self._peak_rate * self.torque ** 2
        
        self.motor.rest_angle = target_angle * math.pi / 180
        self.motor.stiffness = self.torque * self._max_p
        
        if self.torque > 0:
            coeff = np.clip(0.003 / self.torque ** 2, 0.001, 1)
            self.motor.damping = self.motor.stiffness * coeff
        else:
            self.motor.damping = self._max_p / 10000
    
    
    def getAngle(self):
        clp = np.clip(self.body_a.angle - self.body_b.angle, -math.pi, math.pi)
        
        return clp / math.pi * 180


    # e + (c * (1 - e) - s * t**2 * (1 - math.exp(-4 * e))**2) * dt
    def processEnergy(self, dt):
        self.recovery_rate = self._restore_rate * (1.0 - self.energy) + 0.1
        
        self.energy += (self.recovery_rate - self.spend_rate) * dt
        self.energy = np.clip(self.energy, 0.0, 1.0)



class BoxObject:
    def __init__(self, root, xy, ss, mass = 1.0, color = (0, 0, 0, 0)):
        self.root = root
        
        self.body = pymunk.Body(mass, pymunk.moment_for_box(mass, ss))
        self.body.position = xy
        
        self.shape = pymunk.Poly.create_box(self.body, ss)
        
        self.joints = []
        
        self.shape.color = color
        self.root.space.add(self.body, self.shape)
    
    
    def pos(self):
        return self.body.position


    def vel(self):
        return self.body.velocity


    def dist(self, other):
        return self.pos().get_distance(other.pos())
    
    
    def remove(self):
        self.root.space.remove(self.shape)
        
        for j in self.joints: j.remove()
        
        self.root.space.remove(self.body)
    
    
    def setparams(self, mass = 1.0, elastic = 1.0, frict = 1.0):
        self.shape.mass = mass
        self.shape.elasticity = elastic
        self.shape.friction = frict
    
    
    def addJoint(self, other, max_force = 10000, max_angle = (-180, 180)):
        joint = MuscleJoint(self.root, self.body, other.body, max_force, max_angle)
        
        self.joints.append(joint)
        self.root.joints.append(joint)
        
        return len(self.joints) - 1
    
    
    def setTorque(self, target_angle, torque = 1.0, joint_idx = 0):
        self.joints[joint_idx].setTorque(target_angle, torque)
    
    
    def getAngle(self, joint_idx = 0):
        return self.joints[joint_idx].getAngle()


    def getEnergy(self, joint_idx = 0):
        return self.joints[joint_idx].energy


    def getTorque(self, joint_idx = 0):
        return self.joints[joint_idx].torque



class StaticBoxObject:
    def __init__(self, root, xy, ss, mass = 1.0, color = (0, 0, 0, 0)):
        self.root = root
        
        moment = pymunk.moment_for_box(mass, ss)
        self.body = pymunk.Body(mass, moment, pymunk.Body.STATIC)
        self.body.position = xy
        
        self.shape = pymunk.Poly.create_box(self.body, ss)
        
        self.joints = []
        
        self.shape.color = color
        self.root.space.add(self.body, self.shape)
    
    
    def pos(self):
        return self.body.position


    def vel(self):
        return self.body.velocity


    def dist(self, other):
        return self.pos().get_distance(other.pos())
    
    
    def remove(self):
        self.root.space.remove(self.shape)
        
        for j in self.joints: j.remove()
        
        self.root.space.remove(self.body)
    
    
    def setparams(self, mass = 1.0, elastic = 1.0, frict = 1.0):
        self.shape.mass = mass
        self.shape.elasticity = elastic
        self.shape.friction = frict
    
    
    def addJoint(self, other, max_force = 10000, max_angle = (-180, 180)):
        joint = MuscleJoint(self.root, self.body, other.body, max_force, max_angle)
        
        self.joints.append(joint)
        self.root.joints.append(joint)
        
        return len(self.joints) - 1
    
    
    def setTorque(self, target_angle, torque = 1.0, joint_idx = 0):
        self.joints[joint_idx].setTorque(target_angle, torque)
    
    
    def getAngle(self, joint_idx = 0):
        return self.joints[joint_idx].getAngle()
    
    
    def getEnergy(self, joint_idx = 0):
        return self.joints[joint_idx].energy


    def getTorque(self, joint_idx = 0):
        return self.joints[joint_idx].torque



class CircleObject:
    def __init__(self, root, xy, radius, mass = 1.0, color = (0, 0, 0, 0)):
        self.root = root
        
        self.body = pymunk.Body(mass, pymunk.moment_for_circle(mass, 0, radius))
        self.body.position = xy
        
        self.shape = pymunk.Circle(self.body, radius)
        
        self.joints = []
        
        self.shape.color = color
        self.root.space.add(self.body, self.shape)
    
    
    def pos(self):
        return self.body.position


    def vel(self):
        return self.body.velocity


    def dist(self, other):
        return self.pos().get_distance(other.pos())
    
    
    def remove(self):
        self.root.space.remove(self.shape)
        
        for j in self.joints: j.remove()
        
        self.root.space.remove(self.body)
    
    
    def setparams(self, mass = 1.0, elastic = 1.0, frict = 1.0):
        self.shape.mass = mass
        self.shape.elasticity = elastic
        self.shape.friction = frict
    
    
    def addJoint(self, other, max_force = 10000, max_angle = (-180, 180)):
        joint = MuscleJoint(self.root, self.body, other.body, max_force, max_angle)
        
        self.joints.append(joint)
        self.root.joints.append(joint)
        
        return len(self.joints) - 1
    
    
    def setTorque(self, target_angle, torque = 1.0, joint_idx = 0):
        self.joints[joint_idx].setTorque(target_angle, torque)
    
    
    def getAngle(self, joint_idx = 0):
        return self.joints[0].getAngle()
    
    
    def getEnergy(self, joint_idx = 0):
        return self.joints[joint_idx].energy


    def getTorque(self, joint_idx = 0):
        return self.joints[joint_idx].torque



class StaticCircleObject:
    def __init__(self, root, xy, radius, mass = 1.0, color = (0, 0, 0, 0)):
        self.root = root
        
        moment = pymunk.moment_for_circle(mass, 0, radius)
        self.body = pymunk.Body(mass, moment, pymunk.Body.STATIC)
        self.body.position = xy
        
        self.shape = pymunk.Circle(self.body, radius)
        
        self.joints = []
        
        self.shape.color = color
        self.root.space.add(self.body, self.shape)
    
    
    def pos(self):
        return self.body.position


    def vel(self):
        return self.body.velocity


    def dist(self, other):
        return self.pos().get_distance(other.pos())
    
    
    def remove(self):
        self.root.space.remove(self.shape)
        
        for j in self.joints: j.remove()
        
        self.root.space.remove(self.body)
    
    
    def setparams(self, mass = 1.0, elastic = 1.0, frict = 1.0):
        self.shape.mass = mass
        self.shape.elasticity = elastic
        self.shape.friction = frict
    
    
    def addJoint(self, other, max_force = 10000, max_angle = (-180, 180)):
        joint = MuscleJoint(self.root, self.body, other.body, max_force, max_angle)
        
        self.joints.append(joint)
        self.root.joints.append(joint)
        
        return len(self.joints) - 1
    
    
    def setTorque(self, target_angle, torque = 1.0, joint_idx = 0):
        self.joints[joint_idx].setTorque(target_angle, torque)
    
    
    def getAngle(self, joint_idx = 0):
        return self.joints[0].getAngle()
    
    
    def getEnergy(self, joint_idx = 0):
        return self.joints[joint_idx].energy
    
    
    def getTorque(self, joint_idx = 0):
        return self.joints[joint_idx].torque



class Worm:
    def __init__(self, root, xy, segs = 2, force = 10000, color = (255, 0, 0, 0)):
        self.root = root
        
        self.segments = []
        
        xy_temp = list(xy)
        for i in range(segs):
            s = BoxObject(root, xy_temp, (50, 50), 1, color)
            s.setparams(1, 0.85, 0.4)
            
            if i != 0: self.segments[i-1].addJoint(s, force, (-50, 50))
            
            xy_temp[0] += 50
            self.segments.append(s)
    
    
    def remove(self):
        for seg in self.segments: seg.remove()


    def head(self) -> BoxObject:
        return self.segments[-1]
    
    
    def getObs(self, apple_pos) -> torch.Tensor:
        positions = []
        for seg in self.segments:
            pos = seg.pos()
            norm_x = np.clip(pos.x / 1000, 0, 1)
            norm_y = np.clip(pos.y / 800, 0, 1)
            positions.extend([norm_x, norm_y])
        
        velocities = []
        for seg in self.segments:
            vel = seg.vel()
            norm_vx = np.clip(vel.x / 500, -1, 1)
            norm_vy = np.clip(vel.y / 500, -1, 1)
            velocities.extend([norm_vx, norm_vy])
        
        angles = []
        for i in range(4):
            angle = self.segments[i].getAngle()
            norm_angle = np.clip(angle / 50, -1, 1)
            angles.append(norm_angle)
        
        head_pos = self.segments[0].pos()
        dx = apple_pos[0] - head_pos.x
        dy = apple_pos[1] - head_pos.y
        
        norm_dx = np.clip(dx / 500, -1, 1)
        norm_dy = np.clip(dy / 500, -1, 1)
        
        euclidean_dist = np.sqrt(dx**2 + dy**2)
        norm_dist = np.clip(euclidean_dist / 500, 0, 1)
        
        res = torch.tensor(np.hstack((
            positions,
            velocities,
            angles,
            [norm_dx, norm_dy, norm_dist]
        )), dtype=torch.float32)
        
        return res



class Env:
    def __init__(self, ss, render = True):
        self.size = ss
        self.is_render = render
        
        if self.is_render:
            self.screen = pygame.display.set_mode(ss)
            self.draw_opts = pymunk.pygame_util.DrawOptions(self.screen)
        
        self.clock = pygame.time.Clock()
        
        self.space = pymunk.Space(True)
        self.space.gravity = (0, 980)
        
        self.substeps = 4
        self.delta = 0
        self.bg = (0, 0, 0)
        self.joints = []
        
        self.running = False
    
    
    def addStatic(self, xy, zw, elast = 0.7, frict = 0.6):
        segment_shape = pymunk.Segment(self.space.static_body, xy, zw, 100)
        segment_shape.elasticity = elast
        segment_shape.friction = frict
        segment_shape.color = (0, 200, 0, 0)
        self.space.add(segment_shape)
    
    
    def addText(self, text, xy, size = 25, color = (0, 0, 0, 0)):
        if self.is_render:
            font = pygame.font.Font(None, size)
            txt = font.render(text, True, color)
            self.screen.blit(txt, xy)
        else:
            print("[TEXT]", text)
    
    
    def mainloop(self, fps = 60):
        self.running = True
        self.delta = 1 / fps
        frame = 0
        
        self.onStart()
        
        while self.running:
            if self.is_render: self.screen.fill(self.bg)
            
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    self.stop()
                
                elif event.type == pygame.KEYDOWN:
                    self.onKeyDown(event.key)
                                
                elif event.type == pygame.KEYUP:
                    self.onKeyUp(event.key)
                    
                elif event.type == pygame.MOUSEBUTTONDOWN:
                    self.onMouseDown(event.button, event.pos)
                
                elif event.type == pygame.MOUSEBUTTONUP:
                    self.onMouseUp(event.button, event.pos)
        
            self.onMainloop(self.delta, frame)
        
            tm = self.delta / self.substeps
            for _ in range(self.substeps):
                for joint in self.joints: joint.processEnergy(tm)
                self.space.step(tm)
            
            if self.is_render:
                self.space.debug_draw(self.draw_opts)
                pygame.display.flip()
            
            frame += 1
            self.delta = self.clock.tick(fps) / 1000
    

    def stop(self):
        self.running = False
    
    def quit(self):
        pygame.quit()
    
    def onStart(self): ...
    def onMainloop(self, dt, frame): ...
    def onKeyUp(self, key): ...
    def onKeyDown(self, key): ...
    def onMouseDown(self, button, pos): ...
    def onMouseUp(self, buton, pos): ...