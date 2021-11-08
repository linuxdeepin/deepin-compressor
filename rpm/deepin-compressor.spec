%define specrelease 1%{?dist}
%if 0%{?openeuler}
%define specrelease 1
%endif

Name:           deepin-compressor
Version:        5.10.0.22
Release:        %{specrelease}
Summary:        A fast and lightweight application for creating and extracting archives
License:        GPLv3+
URL:            https://github.com/linuxdeepin/deepin-devicemanager
Source0:        %{name}-%{version}.tar.gz

BuildRequires: gcc-c++
BuildRequires: cmake
BuildRequires: qt5-devel

BuildRequires: pkgconfig(gsettings-qt)
BuildRequires: pkgconfig(libsecret-1)
BuildRequires: pkgconfig(gio-unix-2.0)
BuildRequires: pkgconfig(disomaster)
BuildRequires: pkgconfig(dtkwidget)
BuildRequires: pkgconfig(dtkgui)
BuildRequires: kf5-kcodecs-devel
BuildRequires: kf5-karchive-devel
BuildRequires: libzip-devel
BuildRequires: libarchive-devel
BuildRequires: minizip-devel
BuildRequires: poppler-cpp-devel
BuildRequires: gtest-devel

Requires: p7zip p7zip-plugins
Requires: lz4-libs
Requires: unrar
Requires: deepin-shortcut-viewer
Requires: lzop

%description
%{summary}.

%prep
%autosetup

%build
export PATH=%{_qt5_bindir}:$PATH
sed -i "s|^cmake_minimum_required.*|cmake_minimum_required(VERSION 3.0)|" $(find . -name "CMakeLists.txt")
sed -i "s|lib/|%_lib/|" CMakeLists.txt
sed -i "s|/usr/lib|%_libdir|" src/source/common/pluginmanager.cpp
mkdir build && pushd build 
%cmake -DCMAKE_BUILD_TYPE=Release ../  -DAPP_VERSION=%{version} -DVERSION=%{version} 
%make_build  
popd

%install
%make_install -C build INSTALL_ROOT="%buildroot"

%files
%doc README.md
%license LICENSE
%{_bindir}/%{name}
%_libdir/%{name}/plugins/*.so
# %{_datadir}/deepin/dde-file-manager/oem-menuextensions/*.desktop
%{_datadir}/icons/hicolor/scalable/apps/%{name}.svg
%{_datadir}/%{name}/translations/*.qm
%{_datadir}/applications/%{name}.desktop
%{_datadir}/mime/packages/%{name}.xml
%{_datadir}/deepin-manual/manual-assets/application/deepin-compressor/archive-manager/*
%{_datadir}/applications/context-menus/*.conf

%changelog
* Thu Apr 16 2021 zhangdingwen <zhangdingwen@uniontech.com> - 5.10.0.22-1
- Initial release for OpenEuler
