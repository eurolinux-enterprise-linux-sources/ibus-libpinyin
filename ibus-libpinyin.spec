%global snapshot 0

Name:       ibus-libpinyin
Version:    1.6.91
Release:    1%{?dist}
Summary:    Intelligent Pinyin engine based on libpinyin for IBus
License:    GPLv2+
Group:      System Environment/Libraries
URL:        https://github.com/libpinyin/ibus-libpinyin
Source0:    http://downloads.sourceforge.net/libpinyin/ibus-libpinyin/%{name}-%{version}.tar.gz
%if %snapshot
Patch0:     ibus-libpinyin-1.7.x-head.patch
%endif

BuildRequires:  gettext-devel
BuildRequires:  intltool
BuildRequires:  libtool
BuildRequires:  pkgconfig
BuildRequires:  sqlite-devel
BuildRequires:  libuuid-devel
BuildRequires:  opencc-devel
BuildRequires:  lua-devel
BuildRequires:  python2-devel
BuildRequires:  desktop-file-utils
BuildRequires:  ibus-devel >= 1.3
BuildRequires:  libpinyin-devel >= 0.9.91

# Requires(post): sqlite

Requires:   ibus >= 1.2.0
Requires:   libpinyin >= 0.9.91
Requires:   libpinyin-data%{?_isa} >= 0.9.91

Obsoletes: ibus-pinyin < 1.4.0-17

%description
It includes a Chinese Pinyin input method and a Chinese ZhuYin (Bopomofo) 
input method based on libpinyin for IBus.

%prep
%setup -q
%if %snapshot
%patch0 -p1 -b .head
%endif

%build
%configure --disable-static \
           --enable-opencc \
           --disable-boost

# make -C po update-gmo
make %{?_smp_mflags}

%check
desktop-file-validate $RPM_BUILD_ROOT%{_datadir}/applications/ibus-setup-libpinyin.desktop
desktop-file-validate $RPM_BUILD_ROOT%{_datadir}/applications/ibus-setup-libbopomofo.desktop

%install
make install DESTDIR=$RPM_BUILD_ROOT INSTALL="install -p"

%find_lang %{name}

%files -f %{name}.lang
%doc AUTHORS COPYING README
%{_datadir}/applications/ibus-setup-libpinyin.desktop
%{_datadir}/applications/ibus-setup-libbopomofo.desktop
%{_libexecdir}/ibus-engine-libpinyin
%{_libexecdir}/ibus-setup-libpinyin
%{_datadir}/ibus-libpinyin/phrases.txt
%{_datadir}/ibus-libpinyin/icons
%{_datadir}/ibus-libpinyin/setup
%{_datadir}/ibus-libpinyin/base.lua
%{_datadir}/ibus-libpinyin/user.lua
%{_datadir}/ibus-libpinyin/db/english.db
%{_datadir}/ibus-libpinyin/db/strokes.db
%dir %{_datadir}/ibus-libpinyin
%dir %{_datadir}/ibus-libpinyin/db
%{_datadir}/ibus/component/*

%changelog
* Sun Apr 28 2013 Peng Wu <pwu@redhat.com> - 1.6.91-1
- Update to 1.6.91

* Tue Mar 19 2013 Peng Wu <pwu@redhat.com> - 1.5.92-1
- Update to 1.5.92

* Mon Mar  4 2013 Peng Wu <pwu@redhat.com> - 1.5.91-1
- Update to 1.5.91

* Thu Feb 14 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.4.93-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_19_Mass_Rebuild

* Tue Nov 27 2012  Peng Wu <pwu@redhat.com> - 1.4.93-4
- Fixes symbol icons

* Tue Nov 20 2012  Peng Wu <pwu@redhat.com> - 1.4.93-3
- Fixes spec file

* Mon Oct 29 2012  Peng Wu <pwu@redhat.com> - 1.4.93-2
- Fixes libpinyin Requires

* Mon Oct 15 2012  Peng Wu <pwu@redhat.com> - 1.4.93-1
- Update to 1.4.93

* Mon Sep 17 2012  Peng Wu <pwu@redhat.com> - 1.4.92-1
- Update to 1.4.92

* Thu Aug 16 2012  Peng Wu <pwu@redhat.com> - 1.4.91-1
- Update to 1.4.91

* Mon Aug 06 2012  Peng Wu <pwu@redhat.com> - 1.4.2-1
- Update to 1.4.2

* Thu Jul 19 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.4.1-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Mon Jul 16 2012  Peng Wu <pwu@redhat.com> - 1.4.1-4
- Fixes obsoletes

* Wed Jul 11 2012  Peng Wu <pwu@redhat.com> - 1.4.1-3
- Update ibus-libpinyin-1.4.x-head.patch

* Tue Jul 10 2012  Peng Wu <pwu@redhat.com> - 1.4.1-2
- Update ibus-libpinyin-1.4.x-head.patch

* Wed Jul 04 2012  Peng Wu <pwu@redhat.com> - 1.4.1-1
- Update to 1.4.1

* Fri Jun 01 2012  Peng Wu <pwu@redhat.com> - 1.4.0-1
- The first version.
