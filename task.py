import dataclasses
from typing import List
import xml.etree.ElementTree as ET
import cv2
import numpy as np
import subprocess


@dataclasses.dataclass
class Camera:
    cx: float
    cy: float

    ncx: float
    nfx: float

    dx: float
    dy: float

    dpx: float
    dpy: float

    focal: float
    kappa1: float

    sx: float

    tx: float
    ty: float

    tz: float

    rx: float
    ry: float
    rz: float

    @classmethod
    def from_str(cls):
        ...

    @classmethod
    def from_csv(cls, file_path: str):
        tree = ET.parse(file_path)
        root = tree.getroot()
        intrinsic = root.find('Intrinsic').attrib
        intrinsic = {key: float(val) for key, val in intrinsic.items()}

        geometry = root.find('Geometry').attrib
        geometry = {key: float(val) for key, val in geometry.items()}

        extrinsic = root.find('Extrinsic').attrib
        extrinsic = {key: float(val) for key, val in extrinsic.items()}

        return cls(cx=intrinsic['cx'], cy=intrinsic['cy'],
                   focal=intrinsic['focal'],
                   kappa1=intrinsic['kappa1'],
                   sx=intrinsic['sx'],
                   ncx=geometry['ncx'],
                   nfx=geometry['nfx'],
                   dx=geometry['dx'],
                   dy=geometry['dy'],
                   dpx=geometry['dpx'],
                   dpy=geometry['dpy'],
                   tx=extrinsic['tx'],
                   ty=extrinsic['ty'],
                   tz=extrinsic['tz'],
                   rx=extrinsic['rx'],
                   ry=extrinsic['ry'],
                   rz=extrinsic['rz'])

    def get_camera_parameters(self):
        return f'{self.ncx},{self.nfx},{self.dx},{self.dy},{self.dpx},{self.dpy},{self.cx},{self.cy},{self.sx}'

    def get_calibration_constants(self):
        return f'{self.focal},{self.kappa1},0.0,0.0,{self.tx},{self.ty},{self.tz},{self.rx},{self.ty},{self.rz}' \
               f',0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0'


def cast(focal: [float, float], principal: List[float], coords: List[float]):
    x = focal[0] / coords[2] * coords[0] + principal[0]
    y = focal[1] / coords[2] * coords[1] + principal[1]
    return x, y


def chunks(lst: List, n: int):
    for i in range(0, len(lst), n):
        yield lst[i:i + n]


def read_file(file_path: str, seq_path: str, camera: Camera):
    start = 130
    with open(file_path, 'r') as file:
        lines = file.readlines()[1:]
        for line in lines:
            idx, *coords = line.split(';')
            coords = list(chunks(coords, 3))
            casted = [camera.cast([float(ch) for ch in chunk]) for chunk in coords]
            img = cv2.imread(f'{seq_path}/0{start + int(idx)}', 0)
            for x, y in casted:
                img[x, y] = [0, 0, 255]
            cv2.imshow('image', img)
            ...


def read_file_npy(file_path: str, seq_path: str, camera: Camera):
    start = 130
    arr = np.load(file_path, allow_pickle=True)[0]
    for idx, coords in enumerate(arr):
        casted = [camera.cast(chunk) for chunk in coords]
        img = cv2.imread(f'{seq_path}/0{start + int(idx)}.png', cv2.IMREAD_COLOR)
        cv2.imshow('image', img)
        cv2.waitKey(0)
        for x, y in casted:
            img[abs(int(x / 10)), abs(int(y / 10))] = [0, 0, 255]
        cv2.imshow('image', img)
        cv2.waitKey(0)
        ...


def example(path_img, path_):
    ...


def translate_to_csv(file_path: str):
    arr = np.load(file_path, allow_pickle=True)
    for line in arr:
        ...


def run_cpp(prog: str, file_path: str, camera: Camera):
    args = f"{prog} {file_path} {camera.get_camera_parameters()} {camera.get_calibration_constants()}"
    print(args)
    subprocess.call(args, shell=False)


if __name__ == '__main__':
    a = "9;-1452.85;2186.34;966.856;87.9573;28.1164;-2.65344;4.10748;0;4.21633;5.25645;0;0;5.78604;0;6.76691;5.63856;0;0;8.63007;1.86614;5.40711;12.9194;-4.13434;1.09512;-3.72624;3.79311;-82.1212;0;-24.176;0;13.7303;12.3857;71.651;0;30.8861;0;262.391;1713.66;0;-0.89815;1.22122;0.69644;-2.18465;1.19311;1.7172"

    prefix = 'C:/Users/xgg/Projects/Coding/CV_proj'

    path = "Gait_Data/Gait_Data/MoCap/Csv/Full_Sequences/Clothes1/p2s1.csv"
    calib_path = "gait3d-001/Sequences/p2s1/Calibration/c1.xml"
    src_path = "./gait3d-001/Sequences/p2s1/Edges/c1"
    camera = Camera.from_csv(f'{prefix}/{calib_path}')
    run_cpp('cmake-build-debug/cpp_cv.exe', 'C:/Users/xgg/CLionProjects/cpp_cv/p1s1_2.csv', camera)

    # path_npy = "result.npy"
    # read_file_npy(path_npy, src_path, camera)

    # translate_to_csv(f'{prefix}/{path_npy}')
    ...
