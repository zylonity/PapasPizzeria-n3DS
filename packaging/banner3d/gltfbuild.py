import json, base64, struct, math

class GLTF:
    def __init__(self):
        self.bin=bytearray()
        self.bufferViews=[]; self.accessors=[]; self.meshes=[]; self.nodes=[]
        self.materials=[]; self.textures=[]; self.images=[]; self.samplers=[]
        self.cameras=[]; self.animations=[]
    def _bv(self, data, target=None):
        # 4-byte align
        while len(self.bin)%4: self.bin.append(0)
        off=len(self.bin); self.bin+=data
        bv={"buffer":0,"byteOffset":off,"byteLength":len(data)}
        if target: bv["target"]=target
        self.bufferViews.append(bv); return len(self.bufferViews)-1
    def acc_vec3(self, verts):
        data=b"".join(struct.pack("<3f",*v) for v in verts)
        bv=self._bv(data,34962)
        mn=[min(v[i] for v in verts) for i in range(3)]
        mx=[max(v[i] for v in verts) for i in range(3)]
        self.accessors.append({"bufferView":bv,"componentType":5126,"count":len(verts),"type":"VEC3","min":mn,"max":mx})
        return len(self.accessors)-1
    def acc_vec2(self, uvs):
        data=b"".join(struct.pack("<2f",*v) for v in uvs)
        bv=self._bv(data,34962)
        self.accessors.append({"bufferView":bv,"componentType":5126,"count":len(uvs),"type":"VEC2"})
        return len(self.accessors)-1
    def acc_scalar_u16(self, idx):
        data=b"".join(struct.pack("<H",i) for i in idx)
        bv=self._bv(data,34963)
        self.accessors.append({"bufferView":bv,"componentType":5123,"count":len(idx),"type":"SCALAR"})
        return len(self.accessors)-1
    def acc_scalar_f(self, vals):
        data=b"".join(struct.pack("<f",v) for v in vals)
        bv=self._bv(data)
        self.accessors.append({"bufferView":bv,"componentType":5126,"count":len(vals),"type":"SCALAR","min":[min(vals)],"max":[max(vals)]})
        return len(self.accessors)-1
    def acc_vec3_anim(self, vecs):
        data=b"".join(struct.pack("<3f",*v) for v in vecs)
        bv=self._bv(data)
        self.accessors.append({"bufferView":bv,"componentType":5126,"count":len(vecs),"type":"VEC3"})
        return len(self.accessors)-1
    def image(self, uri):
        self.images.append({"uri":uri}); 
        self.samplers.append({"magFilter":9729,"minFilter":9729,"wrapS":33071,"wrapT":33071})
        self.textures.append({"source":len(self.images)-1,"sampler":len(self.samplers)-1})
        return len(self.textures)-1
    def material(self, tex, name, mask=True):
        m={"name":name,
           "pbrMetallicRoughness":{"baseColorTexture":{"index":tex},"metallicFactor":0,"roughnessFactor":1},
           "doubleSided":True}
        if mask: m["alphaMode"]="MASK"; m["alphaCutoff"]=0.5
        else: m["alphaMode"]="BLEND"
        self.materials.append(m); return len(self.materials)-1
    def quad(self, w, h, mat, name="mesh"):
        # centered quad in XY plane, facing +Z, textured (0,0 top-left)
        verts=[(-w,-h,0),(w,-h,0),(w,h,0),(-w,h,0)]
        uvs=[(0,1),(1,1),(1,0),(0,0)]
        norms=[(0,0,1)]*4          # face the camera so the default -Z light lights them
        idx=[0,1,2,0,2,3]
        p=self.acc_vec3(verts); t=self.acc_vec2(uvs); n=self.acc_vec3(norms); i=self.acc_scalar_u16(idx)
        self.meshes.append({"name":name,"primitives":[{"attributes":{"POSITION":p,"NORMAL":n,"TEXCOORD_0":t},"indices":i,"material":mat}]})
        return len(self.meshes)-1
    def node(self, mesh=None, translation=None, name=None, camera=None, children=None):
        n={}
        if name: n["name"]=name
        if mesh is not None: n["mesh"]=mesh
        if camera is not None: n["camera"]=camera
        if translation: n["translation"]=list(translation)
        if children: n["children"]=children
        self.nodes.append(n); return len(self.nodes)-1

    def add_anim(self, channels):
        # channels: list of (node_id, times[list], positions[list of (x,y,z)])
        chs=[]; smps=[]
        for node_id, times, positions in channels:
            ti=self.acc_scalar_f(times)
            po=self.acc_vec3_anim(positions)
            smps.append({"input":ti,"output":po,"interpolation":"LINEAR"})
            chs.append({"sampler":len(smps)-1,"target":{"node":node_id,"path":"translation"}})
        self.animations.append({"name":"orbit","channels":chs,"samplers":smps})

    def save(self, path, root_nodes):
        b64=base64.b64encode(bytes(self.bin)).decode()
        gltf={"asset":{"version":"2.0"},
              "scene":0,"scenes":[{"nodes":root_nodes}],
              "nodes":self.nodes,"meshes":self.meshes,"materials":self.materials,
              "textures":self.textures,"images":self.images,"samplers":self.samplers,
              "accessors":self.accessors,"bufferViews":self.bufferViews,
              "buffers":[{"byteLength":len(self.bin),"uri":"data:application/octet-stream;base64,"+b64}]}
        if self.cameras: gltf["cameras"]=self.cameras
        if self.animations: gltf["animations"]=self.animations
        json.dump(gltf, open(path,"w"))
