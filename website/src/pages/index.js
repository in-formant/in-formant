import React from 'react';
import clsx from 'clsx';
import Layout from '@theme/Layout';
import Link from '@docusaurus/Link';
import useDocusaurusContext from '@docusaurus/useDocusaurusContext';
import useBaseUrl from '@docusaurus/useBaseUrl';
import styles from './styles.module.css';

import Translate, {translate} from '@docusaurus/Translate';

const features = [
  {
    title: translate({
      id: 'homepage.realTimeFeatureTracking.title',
      message: 'Real Time Feature Tracking',
    }),
    // imageUrl: 'img/undraw_docusaurus_mountain.svg',
    description: (
      <Translate id="homepage.realTimeFeatureTracking.content">
        InFormant provides accurate pitch estimation and vocal tract resonance tracking,
        all happening in real time.
      </Translate>
    ),
  },
  {
    title: translate({
      id: 'homepage.powerfulSpectrogram.title',
      message: 'Powerful Spectrogram',
    }),
    // imageUrl: 'img/undraw_docusaurus_tree.svg',
    description: (
      <Translate id="homepage.powerfulSpectrogram.content">
        InFormant directly overlays the estimated pitch and resonance on top of an adaptive
        scrolling spectrogram view.
      </Translate>
    ),
  },
  {
    title: translate({
      id: 'homepage.glottalInverseFiltering.title',
      message: 'Glottal Inverse Filtering',
    }),
    // imageUrl: 'img/undraw_docusaurus_react.svg',
    description: (
      <Translate id="homepage.glottalInverseFiltering.content">
        InFormant also includes an experimental real time glottal inverse filtering algorithm
        that tries to predict the source glottal flow signal from the input signal.
      </Translate>
    ),
  },
];

function Feature({imageUrl, title, description}) {
  const imgUrl = useBaseUrl(imageUrl);
  return (
    <div className={clsx('col col--4', styles.feature)}>
      {imgUrl && (
        <div className="text--center">
          <img className={styles.featureImage} src={imgUrl} alt={title} />
        </div>
      )}
      <h3>{title}</h3>
      <p>{description}</p>
    </div>
  );
}

function Home() {
  const context = useDocusaurusContext();
  const {siteConfig = {}} = context;
  return (
    <Layout
      title={`Documentation`}
      description={translate({
        id: 'homepage.description',
        message: "Homepage for the InFormant documentation",
      })}>
      <header className={clsx('hero hero--primary', styles.heroBanner)}>
        <div className="container">
          <h1 className="hero__title">{siteConfig.title}</h1>
          <p className="hero__subtitle">{siteConfig.tagline}</p>
          <p className="hero__subtitle"><strong>THIS IS STILL UNDER CONSTRUCTION, IMAGES MISSING</strong></p>
          <div className={styles.buttons}>
            <Link
              className={clsx(
                'button button--outline button--secondary button--lg',
                styles.getStarted,
              )}
              to={useBaseUrl('docs/')}>
              <Translate id="homepage.getStarted">
                Get Started
              </Translate>
            </Link>
          </div>
        </div>
      </header>
      <main>
        {features && features.length > 0 && (
          <section className={styles.features}>
            <div className="container">
              <div className="row">
                {features.map((props, idx) => (
                  <Feature key={idx} {...props} />
                ))}
              </div>
            </div>
          </section>
        )}
      </main>
    </Layout>
  );
}

export default Home;
